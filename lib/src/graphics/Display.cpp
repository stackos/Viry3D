/*
* Viry3D
* Copyright 2014-2018 by Stack - stackos@qq.com
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "Display.h"
#include "Application.h"
#include "Debug.h"
#include "RenderState.h"
#include "Color.h"
#include "BufferObject.h"
#include "Camera.h"
#include "Texture.h"
#include "Shader.h"
#include "Mesh.h"
#include "Material.h"
#include "MeshRenderer.h"
#include "container/List.h"
#include "string/String.h"
#include "memory/Memory.h"
#include "math/Matrix4x4.h"
#include "io/File.h"
#include "Debug.h"

extern "C"
{
#include "crypto/md5/md5.h"
}

#ifdef max
#undef max
#endif

#if VR_ANDROID
#include "android/jni.h"
#endif

#if VR_VULKAN
#include "vulkan/spirv_cross/spirv_glsl.hpp"

#if VR_WINDOWS || VR_ANDROID
#include "vulkan/vulkan_shader_compiler.h"
#elif VR_IOS || VR_MAC
#include "GLSLConversion.h"
#endif

static PFN_vkGetDeviceProcAddr g_gdpa = nullptr;

#define GET_INSTANCE_PROC_ADDR(inst, entrypoint)                                                                \
    {                                                                                                           \
        fp##entrypoint = (PFN_vk##entrypoint) vkGetInstanceProcAddr(inst, "vk" #entrypoint);                    \
    }

#define GET_DEVICE_PROC_ADDR(dev, entrypoint)                                                                    \
    {                                                                                                            \
        if (!g_gdpa) g_gdpa = (PFN_vkGetDeviceProcAddr) vkGetInstanceProcAddr(m_instance, "vkGetDeviceProcAddr");\
        fp##entrypoint = (PFN_vk##entrypoint) g_gdpa(dev, "vk" #entrypoint);                                     \
    }

#define VSYNC 0
#define DESCRIPTOR_POOL_SIZE_MAX 65536
#define VERTEX_INPUT_BINDING_VERTEX 0
#define VERTEX_INPUT_BINDING_INSTANCE 1

#elif VR_GLES
#if VR_MAC
#import <AppKit/NSOpenGL.h>
#elif VR_IOS
#import <OpenGLES/EAGL.h>
#endif
#endif

namespace Viry3D
{
#if VR_VULKAN
    static void StringVectorAdd(Vector<char*>& vec, const char* str)
    {
        int size = (int) strlen(str);
        char* str_new = Memory::Alloc<char>(size + 1);
        strcpy(str_new, str);
        vec.Add(str_new);
    }

    static void StringVectorClear(Vector<char*>& vec)
    {
        for (int i = 0; i < vec.Size(); ++i)
        {
            Memory::Free(vec[i]);
        }
        vec.Clear();
    }

    static void GlslToSpirvCached(const String& glsl, VkShaderStageFlagBits shader_type, Vector<unsigned int>& spirv)
    {
        unsigned char hash_bytes[16];
        MD5_CTX md5_context;
        MD5_Init(&md5_context);
        MD5_Update(&md5_context, (void*) glsl.CString(), glsl.Size());
        MD5_Final(hash_bytes, &md5_context);
        String md5_str;
        for (int i = 0; i < sizeof(hash_bytes); ++i)
        {
            md5_str += String::Format("%02x", hash_bytes[i]);
        }

        String cache_path = Application::Instance()->GetSavePath() + "/" + md5_str + ".cache";
        if (File::Exist(cache_path))
        {
            auto buffer = File::ReadAllBytes(cache_path);
            spirv.Resize(buffer.Size() / 4);
            Memory::Copy(&spirv[0], buffer.Bytes(), buffer.Size());
        }
        else
        {
#if VR_WINDOWS || VR_ANDROID
            String error;
            bool success = GlslToSpv(shader_type, glsl.CString(), spirv, error);
            if (!success)
            {
                Log("shader compile error: %s", error.CString());
            }
            assert(success);
#elif VR_IOS || VR_MAC
            MVKShaderStage stage;
            switch (shader_type)
            {
                case VK_SHADER_STAGE_COMPUTE_BIT:
                    stage = kMVKShaderStageCompute;
                    break;
                case VK_SHADER_STAGE_VERTEX_BIT:
                    stage = kMVKShaderStageVertex;
                    break;
                case VK_SHADER_STAGE_FRAGMENT_BIT:
                    stage = kMVKShaderStageFragment;
                    break;
                default:
                    stage = kMVKShaderStageAuto;
                    break;
            }
            uint32_t* spirv_code = nullptr;
            size_t size = 0;
            char* log = nullptr;
            bool success = mvkConvertGLSLToSPIRV(glsl.CString(),
                                       stage,
                                       &spirv_code,
                                       &size,
                                       &log,
                                       true,
                                       true);
            if (!success)
            {
                Log("shader compile error: %s", log);
            }
            assert(success);
            
            spirv.Resize((int) size / 4);
            Memory::Copy(&spirv[0], spirv_code, spirv.SizeInBytes());
            
            free(log);
            free(spirv_code);
#endif
            
            ByteBuffer buffer(spirv.SizeInBytes());
            Memory::Copy(buffer.Bytes(), &spirv[0], buffer.Size());
            File::WriteAllBytes(cache_path, buffer);
        }
    }

    static String ProcessShaderSource(const String& glsl, const String& predefine, const Vector<String>& includes)
    {
        static const String s_shader_header =
            "#version 310 es\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "#define VR_VULKAN 1\n"
            "#define UniformBuffer(set_index, binding_index) layout(std140, set = set_index, binding = binding_index)\n"
            "#define UniformTexture(set_index, binding_index) layout(set = set_index, binding = binding_index)\n"
            "#define Input(location_index) layout(location = location_index) in\n"
            "#define Output(location_index) layout(location = location_index) out\n";

        String source = s_shader_header;
        source += predefine + "\n";

        for (const auto& i : includes)
        {
            auto include_path = Application::Instance()->GetDataPath() + "/shader/Include/" + i;
            auto bytes = File::ReadAllBytes(include_path);
            auto include_str = String(bytes);
            source += include_str + "\n";
        }
        source += glsl;

        return source;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL
        DebugFunc(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType,
            uint64_t srcObject, size_t location, int32_t msgCode,
            const char *pLayerPrefix, const char *pMsg, void *pUserData)
    {
        String message;

        if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        {
            message = String::Format("ERROR: [%s] Code %d : %s", pLayerPrefix, msgCode, pMsg);
        }
        else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        {
            message = String::Format("WARNING: [%s] Code %d : %s", pLayerPrefix, msgCode, pMsg);
        }
        else if (msgFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
        {
            message = String::Format("WARNING: [%s] Code %d : %s", pLayerPrefix, msgCode, pMsg);
        }
        else if (msgFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
        {
            message = String::Format("INFO: [%s] Code %d : %s", pLayerPrefix, msgCode, pMsg);
        }

        Log("%s", message.CString());

#if VR_WINDOWS
        MessageBox(nullptr, message.CString(), "Alert", MB_OK);
#endif

        return false;
    }

    struct SwapchainImageResources
    {
        int width;
        int height;
        VkFormat format;
        VkImage image;
        VkImageView image_view;
        VkCommandBuffer cmd = VK_NULL_HANDLE;
    };
#elif VR_UWP
extern void BindSharedContext();
extern void UnbindSharedContext();
#endif

    class DisplayPrivate
    {
    public:
        static Display* m_display;
        Display* m_public;
        void* m_window;
        int m_width;
        int m_height;
        List<Ref<Camera>> m_cameras;
        Ref<Shader> m_blit_shader;
        Ref<Mesh> m_blit_mesh;
#if VR_VULKAN
        Vector<char*> m_enabled_layers;
        Vector<char*> m_instance_extension_names;
        Vector<char*> m_device_extension_names;
        bool m_has_debug_report_extension = false;
        bool m_has_multiview_extension = false;
        VkInstance m_instance = VK_NULL_HANDLE;
        VkDebugReportCallbackEXT m_debug_callback = VK_NULL_HANDLE;
        VkPhysicalDevice m_gpu = VK_NULL_HANDLE;
        VkSurfaceKHR m_surface = VK_NULL_HANDLE;
        VkDevice m_device = VK_NULL_HANDLE;
        VkQueue m_graphics_queue = VK_NULL_HANDLE;
        VkQueue m_compute_queue = VK_NULL_HANDLE;
        Vector<VkQueueFamilyProperties> m_queue_properties;
        VkPhysicalDeviceMemoryProperties m_memory_properties;
        VkPhysicalDeviceFeatures m_gpu_features;
        VkPhysicalDeviceProperties m_gpu_properties;
        PFN_vkCreateDebugReportCallbackEXT fpCreateDebugReportCallbackEXT = nullptr;
        PFN_vkDestroyDebugReportCallbackEXT fpDestroyDebugReportCallbackEXT = nullptr;
        PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR = nullptr;
        PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
        PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR = nullptr;
        PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR = nullptr;
        PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR = nullptr;
        PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR = nullptr;
        PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR = nullptr;
        PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR = nullptr;
        PFN_vkQueuePresentKHR fpQueuePresentKHR = nullptr;
        int m_graphics_queue_family_index = -1;
        int m_compute_queue_family_index = -1;
        VkSurfaceFormatKHR m_surface_format;
        VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
        Vector<SwapchainImageResources> m_swapchain_image_resources;
        VkFence m_image_fence = VK_NULL_HANDLE;
        VkFence m_draw_complete_fence = VK_NULL_HANDLE;
        VkSemaphore m_image_acquired_semaphore = VK_NULL_HANDLE;
        VkSemaphore m_compute_semaphore = VK_NULL_HANDLE;
        VkSemaphore m_draw_complete_semaphore = VK_NULL_HANDLE;
        int m_image_index = 0;
        VkCommandPool m_graphics_cmd_pool = VK_NULL_HANDLE;
        VkCommandPool m_image_cmd_pool = VK_NULL_HANDLE;
        VkCommandBuffer m_image_cmd = VK_NULL_HANDLE;
        VkCommandPool m_compute_cmd_pool = VK_NULL_HANDLE;
        VkCommandBuffer m_compute_cmd = VK_NULL_HANDLE;
        bool m_has_compute_instance_cmds = false;
        Mutex m_image_cmd_mutex;
        Ref<Texture> m_depth_texture;
        bool m_primary_cmd_dirty = true;
        bool m_pause_draw = false;
#endif

        DisplayPrivate(Display* display, void* window, int width, int height):
            m_public(display),
            m_window(window),
            m_width(width),
            m_height(height)
        {
            m_display = m_public;
            
#if VR_GLES
            this->InitContext();
#endif
        }

#if VR_VULKAN
        ~DisplayPrivate()
        {
            vkDeviceWaitIdle(m_device);

            m_blit_mesh.reset();
            m_blit_shader.reset();
            m_cameras.Clear();

            this->DestroySizeDependentResources();

            if (m_compute_cmd_pool != VK_NULL_HANDLE)
            {
                if (m_compute_cmd != VK_NULL_HANDLE)
                {
                    vkFreeCommandBuffers(m_device, m_compute_cmd_pool, 1, &m_compute_cmd);
                    m_compute_cmd = VK_NULL_HANDLE;
                }
                vkDestroyCommandPool(m_device, m_compute_cmd_pool, nullptr);
                m_compute_cmd_pool = VK_NULL_HANDLE;
            }
            vkFreeCommandBuffers(m_device, m_image_cmd_pool, 1, &m_image_cmd);
            vkDestroyCommandPool(m_device, m_image_cmd_pool, nullptr);
            vkDestroyFence(m_device, m_image_fence, nullptr);
            vkDestroyFence(m_device, m_draw_complete_fence, nullptr);
            vkDestroySemaphore(m_device, m_image_acquired_semaphore, nullptr);
            vkDestroySemaphore(m_device, m_compute_semaphore, nullptr);
            vkDestroySemaphore(m_device, m_draw_complete_semaphore, nullptr);
            vkDestroyDevice(m_device, nullptr);
            if (m_surface != VK_NULL_HANDLE)
            {
                vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
                m_surface = VK_NULL_HANDLE;
            }
            m_gpu = VK_NULL_HANDLE;
            if (m_debug_callback != VK_NULL_HANDLE)
            {
                fpDestroyDebugReportCallbackEXT(m_instance, m_debug_callback, nullptr);
                m_debug_callback = VK_NULL_HANDLE;
            }
            vkDestroyInstance(m_instance, nullptr);
            StringVectorClear(m_enabled_layers);
            StringVectorClear(m_instance_extension_names);
            StringVectorClear(m_device_extension_names);

            m_public = nullptr;

            m_display = nullptr;
        }

        void CheckInstanceLayers()
        {
            uint32_t instance_layer_count;
            VkResult err = vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr);
            assert(!err);
            assert(instance_layer_count > 0);

            Vector<VkLayerProperties> instance_layers(instance_layer_count);
            err = vkEnumerateInstanceLayerProperties(&instance_layer_count, &instance_layers[0]);
            assert(!err);

            auto check_layer = [](const char** validation_layers, int validation_layer_count, const VkLayerProperties* layers, int layer_count) {
                for (int i = 0; i < validation_layer_count; ++i)
                {
                    bool find = false;

                    for (int j = 0; j < layer_count; ++j)
                    {
                        if (strcmp(validation_layers[i], layers[j].layerName) == 0)
                        {
                            find = true;
                            break;
                        }
                    }

                    if (!find)
                    {
                        return false;
                    }
                }

                return true;
            };

            const char* instance_validation_layers_alt1[] = {
                "VK_LAYER_GOOGLE_threading",
                "VK_LAYER_LUNARG_parameter_validation",
                "VK_LAYER_LUNARG_object_tracker",
                "VK_LAYER_LUNARG_core_validation",
                "VK_LAYER_GOOGLE_unique_objects"
            };
            const char* instance_validation_layers_alt2[] = {
                "VK_LAYER_LUNARG_standard_validation"
            };
            const char* instance_validation_layers_alt3[] = {
                "MoltenVK"
            };

            bool validation_found = false;
            const char** instance_validation_layers = nullptr;
            int instance_validation_layer_count = 0;
            
            if (!validation_found)
            {
                instance_validation_layers = instance_validation_layers_alt1;
                instance_validation_layer_count = sizeof(instance_validation_layers_alt1) / sizeof(instance_validation_layers_alt1[0]);

                validation_found = check_layer(
                    instance_validation_layers,
                    instance_validation_layer_count,
                    &instance_layers[0],
                    instance_layers.Size());
            }
            if (!validation_found)
            {
                instance_validation_layers = instance_validation_layers_alt2;
                instance_validation_layer_count = sizeof(instance_validation_layers_alt2) / sizeof(instance_validation_layers_alt2[0]);
                
                validation_found = check_layer(
                    instance_validation_layers,
                    instance_validation_layer_count,
                    &instance_layers[0],
                    instance_layers.Size());
            }
            if (!validation_found)
            {
                instance_validation_layers = instance_validation_layers_alt3;
                instance_validation_layer_count = sizeof(instance_validation_layers_alt3) / sizeof(instance_validation_layers_alt3[0]);
                
                validation_found = check_layer(
                    instance_validation_layers,
                    instance_validation_layer_count,
                    &instance_layers[0],
                    instance_layers.Size());
            }

            if (validation_found)
            {
                for (int i = 0; i < instance_validation_layer_count; ++i)
                {
                    StringVectorAdd(m_enabled_layers, instance_validation_layers[i]);
                }
            }
        }

        void CheckInstanceExtensions()
        {
            uint32_t instance_extension_count;
            VkResult err = vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr);
            assert(!err);
            assert(instance_extension_count > 0);

            Vector<VkExtensionProperties> instance_extensions(instance_extension_count);
            err = vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, &instance_extensions[0]);
            assert(!err);

            bool surface_ext_found = false;
            bool platform_surface_ext_found = false;

            for (int i = 0; i < instance_extensions.Size(); ++i)
            {
                if (strcmp(VK_KHR_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName) == 0)
                {
                    surface_ext_found = true;
                    StringVectorAdd(m_instance_extension_names, VK_KHR_SURFACE_EXTENSION_NAME);
                }
#if defined(VR_WINDOWS)
                if (strcmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName) == 0)
                {
                    platform_surface_ext_found = true;
                    StringVectorAdd(m_instance_extension_names, VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
                }
#elif defined(VR_ANDROID)
                if (strcmp(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName) == 0)
                {
                    platform_surface_ext_found = true;
                    StringVectorAdd(m_instance_extension_names, VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
                }
#elif defined(VR_IOS)
                if (strcmp(VK_MVK_IOS_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName) == 0)
                {
                    platform_surface_ext_found = true;
                    StringVectorAdd(m_instance_extension_names, VK_MVK_IOS_SURFACE_EXTENSION_NAME);
                }
#elif defined(VR_MAC)
                if (strcmp(VK_MVK_MACOS_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName) == 0)
                {
                    platform_surface_ext_found = true;
                    StringVectorAdd(m_instance_extension_names, VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
                }
#endif
                if (strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, instance_extensions[i].extensionName) == 0)
                {
                    m_has_debug_report_extension = true;
                    StringVectorAdd(m_instance_extension_names, VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
                }
                if (strcmp(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, instance_extensions[i].extensionName) == 0)
                {
                    StringVectorAdd(m_instance_extension_names, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
                }
            }

            assert(surface_ext_found);
            assert(platform_surface_ext_found);
        }

        void CreateInstance(const String& name)
        {
            VkApplicationInfo app_info;
            Memory::Zero(&app_info, sizeof(app_info));
            app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            app_info.pNext = nullptr;
            app_info.pApplicationName = name.CString();
            app_info.applicationVersion = 0;
            app_info.pEngineName = name.CString();
            app_info.engineVersion = 0;
            app_info.apiVersion = VK_API_VERSION_1_0;

            VkInstanceCreateInfo inst_info;
            Memory::Zero(&inst_info, sizeof(inst_info));
            inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            inst_info.pNext = nullptr;
            inst_info.flags = 0;
            inst_info.pApplicationInfo = &app_info;
            if (m_enabled_layers.Size() > 0)
            {
                inst_info.enabledLayerCount = m_enabled_layers.Size();
                inst_info.ppEnabledLayerNames = &m_enabled_layers[0];
            }
            else
            {
                inst_info.enabledLayerCount = 0;
                inst_info.ppEnabledLayerNames = nullptr;
            }
            inst_info.enabledExtensionCount = m_instance_extension_names.Size();
            inst_info.ppEnabledExtensionNames = &m_instance_extension_names[0];

            VkResult err = vkCreateInstance(&inst_info, nullptr, &m_instance);
            assert(!err);

            GET_INSTANCE_PROC_ADDR(m_instance, CreateDebugReportCallbackEXT);
            GET_INSTANCE_PROC_ADDR(m_instance, DestroyDebugReportCallbackEXT);
            GET_INSTANCE_PROC_ADDR(m_instance, GetPhysicalDeviceSurfaceSupportKHR);
            GET_INSTANCE_PROC_ADDR(m_instance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
            GET_INSTANCE_PROC_ADDR(m_instance, GetPhysicalDeviceSurfaceFormatsKHR);
            GET_INSTANCE_PROC_ADDR(m_instance, GetPhysicalDeviceSurfacePresentModesKHR);
            GET_INSTANCE_PROC_ADDR(m_instance, GetSwapchainImagesKHR);
        }

        void CreateDebugReportCallback()
        {
            if (m_has_debug_report_extension)
            {
                VkDebugReportCallbackCreateInfoEXT create_info;
                Memory::Zero(&create_info, sizeof(create_info));
                create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
                create_info.pNext = nullptr;
                create_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
                create_info.pfnCallback = DebugFunc;
                create_info.pUserData = nullptr;
                
                VkResult err = fpCreateDebugReportCallbackEXT(m_instance, &create_info, nullptr, &m_debug_callback);
                assert(!err);
            }
        }

        void InitPhysicalDevice()
        {
            uint32_t gpu_count;
            VkResult err = vkEnumeratePhysicalDevices(m_instance, &gpu_count, nullptr);
            assert(!err);
            assert(gpu_count > 0);

            Vector<VkPhysicalDevice> physical_devices(gpu_count);
            err = vkEnumeratePhysicalDevices(m_instance, &gpu_count, &physical_devices[0]);
            assert(!err);

            m_gpu = physical_devices[0];

            uint32_t device_extension_count;
            err = vkEnumerateDeviceExtensionProperties(m_gpu, nullptr, &device_extension_count, nullptr);
            assert(!err);
            assert(device_extension_count > 0);

            Vector<VkExtensionProperties> device_extensions(device_extension_count);
            err = vkEnumerateDeviceExtensionProperties(m_gpu, nullptr, &device_extension_count, &device_extensions[0]);
            assert(!err);

            bool swapchain_ext_found = false;
            for (int i = 0; i < device_extensions.Size(); ++i)
            {
                if (strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME, device_extensions[i].extensionName) == 0)
                {
                    swapchain_ext_found = true;
                    StringVectorAdd(m_device_extension_names, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
                }
                else if (strcmp(VK_KHR_MULTIVIEW_EXTENSION_NAME, device_extensions[i].extensionName) == 0)
                {
                    m_has_multiview_extension = true;
                    StringVectorAdd(m_device_extension_names, VK_KHR_MULTIVIEW_EXTENSION_NAME);
                }
            }
            assert(swapchain_ext_found);

            uint32_t queue_family_count;
            vkGetPhysicalDeviceQueueFamilyProperties(m_gpu, &queue_family_count, nullptr);
            assert(queue_family_count > 0);

            m_queue_properties.Resize(queue_family_count);
            vkGetPhysicalDeviceQueueFamilyProperties(m_gpu, &queue_family_count, &m_queue_properties[0]);

            // compute queue
            bool has_compute_queue = false;
            for (int i = 0; i < m_queue_properties.Size(); ++i)
            {
                if ((m_queue_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && ((m_queue_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
                {
                    m_compute_queue_family_index = i;
                    has_compute_queue = true;
                    break;
                }
            }
            if (!has_compute_queue)
            {
                for (int i = 0; i < m_queue_properties.Size(); ++i)
                {
                    if ((m_queue_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT))
                    {
                        m_compute_queue_family_index = i;
                        has_compute_queue = true;
                        break;
                    }
                }
            }

            vkGetPhysicalDeviceMemoryProperties(m_gpu, &m_memory_properties);
            vkGetPhysicalDeviceFeatures(m_gpu, &m_gpu_features);
            vkGetPhysicalDeviceProperties(m_gpu, &m_gpu_properties);
        }

        void CreateSurface()
        {
            VkResult err;

#if VR_WINDOWS
            VkWin32SurfaceCreateInfoKHR create_info;
            Memory::Zero(&create_info, sizeof(create_info));
            create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
            create_info.pNext = nullptr;
            create_info.flags = 0;
            create_info.hinstance = GetModuleHandle(nullptr);
            create_info.hwnd = (HWND) m_window;

            err = vkCreateWin32SurfaceKHR(m_instance, &create_info, nullptr, &m_surface);
#elif VR_ANDROID
            VkAndroidSurfaceCreateInfoKHR create_info;
            Memory::Zero(&create_info, sizeof(create_info));
            create_info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
            create_info.pNext = nullptr;
            create_info.flags = 0;
            create_info.window = (ANativeWindow*) get_native_window();

            err = vkCreateAndroidSurfaceKHR(m_instance, &create_info, nullptr, &m_surface);
#elif VR_IOS
            VkIOSSurfaceCreateInfoMVK create_info;
            Memory::Zero(&create_info, sizeof(create_info));
            create_info.sType = VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK;
            create_info.pNext = nullptr;
            create_info.flags = 0;
            create_info.pView = m_window;

            err = vkCreateIOSSurfaceMVK(m_instance, &create_info, nullptr, &m_surface);
#elif VR_MAC
            VkMacOSSurfaceCreateInfoMVK create_info;
            Memory::Zero(&create_info, sizeof(create_info));
            create_info.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
            create_info.pNext = nullptr;
            create_info.flags = 0;
            create_info.pView = m_window;

            err = vkCreateMacOSSurfaceMVK(m_instance, &create_info, nullptr, &m_surface);
#endif
            assert(!err);

            Vector<VkBool32> present_supports(m_queue_properties.Size());
            for (int i = 0; i < present_supports.Size(); ++i)
            {
                fpGetPhysicalDeviceSurfaceSupportKHR(m_gpu, i, m_surface, &present_supports[i]);
            }

            int graphics_queue_index = -1;
            int present_queue_index = -1;
            for (int i = 0; i < m_queue_properties.Size(); ++i)
            {
                if ((m_queue_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
                {
                    if (graphics_queue_index < 0)
                    {
                        graphics_queue_index = i;
                    }

                    if (present_supports[i] == VK_TRUE)
                    {
                        graphics_queue_index = i;
                        present_queue_index = i;
                        break;
                    }
                }
            }

            if (present_queue_index < 0)
            {
                for (int i = 0; i < m_queue_properties.Size(); ++i)
                {
                    if (present_supports[i] == VK_TRUE)
                    {
                        present_queue_index = i;
                        break;
                    }
                }
            }

            assert(graphics_queue_index >= 0 && present_queue_index >= 0);
            assert(graphics_queue_index == present_queue_index);

            m_graphics_queue_family_index = graphics_queue_index;

            uint32_t surface_format_count;
            err = fpGetPhysicalDeviceSurfaceFormatsKHR(m_gpu, m_surface, &surface_format_count, nullptr);
            assert(!err);

            Vector<VkSurfaceFormatKHR> surface_formats(surface_format_count);
            err = fpGetPhysicalDeviceSurfaceFormatsKHR(m_gpu, m_surface, &surface_format_count, &surface_formats[0]);
            assert(!err);

            if (surface_format_count == 1 && surface_formats[0].format == VK_FORMAT_UNDEFINED)
            {
                m_surface_format.format = VK_FORMAT_B8G8R8A8_UNORM;
            }
            else
            {
                assert(surface_format_count > 0);
                m_surface_format.format = surface_formats[0].format;
            }
            m_surface_format.colorSpace = surface_formats[0].colorSpace;
        }

        void CreateDevice()
        {
            VkResult err;

            float queue_priority = 0.0f;
            Vector<VkDeviceQueueCreateInfo> queue_infos;
            VkDeviceQueueCreateInfo queue_info;
            Memory::Zero(&queue_info, sizeof(queue_info));
            queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_info.pNext = nullptr;
            queue_info.flags = 0;
            queue_info.queueFamilyIndex = m_graphics_queue_family_index;
            queue_info.queueCount = 1;
            queue_info.pQueuePriorities = &queue_priority;
            queue_infos.Add(queue_info);

            if (m_compute_queue_family_index >= 0 && m_compute_queue_family_index != m_graphics_queue_family_index)
            {
                queue_info.queueFamilyIndex = m_compute_queue_family_index;
                queue_infos.Add(queue_info);
            }

            VkDeviceCreateInfo device_info;
            Memory::Zero(&device_info, sizeof(device_info));
            device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            device_info.pNext = nullptr;
            device_info.flags = 0;
            device_info.queueCreateInfoCount = queue_infos.Size();
            device_info.pQueueCreateInfos = &queue_infos[0];
            device_info.enabledLayerCount = m_enabled_layers.Size();
            device_info.ppEnabledLayerNames = &m_enabled_layers[0];
            device_info.enabledExtensionCount = m_device_extension_names.Size();
            device_info.ppEnabledExtensionNames = &m_device_extension_names[0];
            device_info.pEnabledFeatures = nullptr;

            err = vkCreateDevice(m_gpu, &device_info, nullptr, &m_device);
            assert(!err);

            GET_DEVICE_PROC_ADDR(m_device, CreateSwapchainKHR);
            GET_DEVICE_PROC_ADDR(m_device, DestroySwapchainKHR);
            GET_DEVICE_PROC_ADDR(m_device, GetSwapchainImagesKHR);
            GET_DEVICE_PROC_ADDR(m_device, AcquireNextImageKHR);
            GET_DEVICE_PROC_ADDR(m_device, QueuePresentKHR);
        }

        void GetQueues()
        {
            vkGetDeviceQueue(m_device, m_graphics_queue_family_index, 0, &m_graphics_queue);

            if (m_compute_queue_family_index >= 0)
            {
                vkGetDeviceQueue(m_device, m_compute_queue_family_index, 0, &m_compute_queue);
            }
        }

        void CreateSignals()
        {
            VkFenceCreateInfo fence_info;
            Memory::Zero(&fence_info, sizeof(fence_info));
            fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fence_info.pNext = nullptr;
            fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            VkSemaphoreCreateInfo semaphore_info;
            Memory::Zero(&semaphore_info, sizeof(semaphore_info));
            semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            semaphore_info.pNext = nullptr;
            semaphore_info.flags = 0;

            VkResult err = vkCreateFence(m_device, &fence_info, nullptr, &m_image_fence);
            assert(!err);
            err = vkCreateFence(m_device, &fence_info, nullptr, &m_draw_complete_fence);
            assert(!err);
            err = vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_image_acquired_semaphore);
            assert(!err);
            err = vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_compute_semaphore);
            assert(!err);
            err = vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_draw_complete_semaphore);
            assert(!err);
        }

        void CreateImageCmd()
        {
            this->CreateCommandPool(m_graphics_queue_family_index, &m_image_cmd_pool);
            this->CreateCommandBuffer(m_image_cmd_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, &m_image_cmd);
        }

        void CreateComputeCmd()
        {
            if (m_compute_queue_family_index >= 0)
            {
                this->CreateCommandPool(m_compute_queue_family_index, &m_compute_cmd_pool);
                this->CreateCommandBuffer(m_compute_cmd_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, &m_compute_cmd);
            }
        }

        void CreateSizeDependentResources()
        {
            this->CreateSwapChain();
            this->CreateCommandPool(m_graphics_queue_family_index, &m_graphics_cmd_pool);
            for (int i = 0; i < m_swapchain_image_resources.Size(); ++i)
            {
                this->CreateCommandBuffer(m_graphics_cmd_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, &m_swapchain_image_resources[i].cmd);
            }
            m_depth_texture = Texture::CreateRenderTexture(
                m_width,
                m_height,
                Texture::ChooseDepthFormatSupported(false),
                1,
                1,
                false,
                FilterMode::None,
                SamplerAddressMode::None);
        }

        void DestroySizeDependentResources()
        {
            m_depth_texture.reset();

            for (int i = 0; i < m_swapchain_image_resources.Size(); ++i)
            {
                vkFreeCommandBuffers(m_device, m_graphics_cmd_pool, 1, &m_swapchain_image_resources[i].cmd);
            }
            if (m_graphics_cmd_pool != VK_NULL_HANDLE)
            {
                vkDestroyCommandPool(m_device, m_graphics_cmd_pool, nullptr);
                m_graphics_cmd_pool = VK_NULL_HANDLE;
            }

            for (int i = 0; i < m_swapchain_image_resources.Size(); ++i)
            {
                vkDestroyImageView(m_device, m_swapchain_image_resources[i].image_view, nullptr);
            }
            m_swapchain_image_resources.Clear();
            if (m_swapchain != VK_NULL_HANDLE)
            {
                fpDestroySwapchainKHR(m_device, m_swapchain, nullptr);
                m_swapchain = VK_NULL_HANDLE;
            }
        }

        void OnResize(int width, int height)
        {
            m_width = width;
            m_height = height;

            vkDeviceWaitIdle(m_device);

            for (auto i : m_cameras)
            {
                i->OnResize(m_width, m_height);
            }

            this->DestroySizeDependentResources();
            vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
            m_surface = VK_NULL_HANDLE;
            this->CreateSurface();
            this->GetQueues();
            this->CreateSizeDependentResources();

            m_primary_cmd_dirty = true;
        }

        void OnPause()
        {
            m_pause_draw = true;

            vkDeviceWaitIdle(m_device);

            for (auto i : m_cameras)
            {
                i->OnPause();
            }

            this->DestroySizeDependentResources();
            vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
            m_surface = VK_NULL_HANDLE;
        }

        void OnResume()
        {
            m_pause_draw = false;

            this->CreateSurface();
            this->GetQueues();
            this->CreateSizeDependentResources();
        }

        void CreateSwapChain()
        {
            VkResult err;

            VkSurfaceCapabilitiesKHR surface_caps;
            err = fpGetPhysicalDeviceSurfaceCapabilitiesKHR(m_gpu, m_surface, &surface_caps);
            assert(!err);

            VkExtent2D swapchain_size;
            swapchain_size.width = (uint32_t) m_width;
            swapchain_size.height = (uint32_t) m_height;

            uint32_t present_mode_count;
            err = fpGetPhysicalDeviceSurfacePresentModesKHR(m_gpu, m_surface, &present_mode_count, nullptr);
            assert(!err);
            assert(present_mode_count > 0);

            Vector<VkPresentModeKHR> present_modes(present_mode_count);
            err = fpGetPhysicalDeviceSurfacePresentModesKHR(m_gpu, m_surface, &present_mode_count, &present_modes[0]);
            assert(!err);

#if (VR_WINDOWS || VR_MAC) && !VSYNC
            VkPresentModeKHR present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
#else
            VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
#endif

            uint32_t swapchain_image_count = 3;
            if (swapchain_image_count < surface_caps.minImageCount)
            {
                swapchain_image_count = surface_caps.minImageCount;
            }

            if ((surface_caps.maxImageCount > 0) && (swapchain_image_count > surface_caps.maxImageCount))
            {
                swapchain_image_count = surface_caps.maxImageCount;
            }

            VkSurfaceTransformFlagBitsKHR transform_flag;
            if (surface_caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
            {
                transform_flag = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
            }
            else
            {
                transform_flag = surface_caps.currentTransform;
            }

            VkCompositeAlphaFlagBitsKHR composite_alpha_flag = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            const int composite_alpha_flag_count = 4;
            VkCompositeAlphaFlagBitsKHR composite_alpha_flags[composite_alpha_flag_count] = {
                VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
                VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
                VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
            };
            for (int i = 0; i < composite_alpha_flag_count; ++i)
            {
                if (surface_caps.supportedCompositeAlpha & composite_alpha_flags[i])
                {
                    composite_alpha_flag = composite_alpha_flags[i];
                    break;
                }
            }

            VkSwapchainKHR old_swapchain = m_swapchain;

            VkSwapchainCreateInfoKHR swapchain_info;
            Memory::Zero(&swapchain_info, sizeof(swapchain_info));
            swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            swapchain_info.pNext = nullptr;
            swapchain_info.flags = 0;
            swapchain_info.surface = m_surface;
            swapchain_info.minImageCount = swapchain_image_count;
            swapchain_info.imageFormat = m_surface_format.format;
            swapchain_info.imageColorSpace = m_surface_format.colorSpace;
            swapchain_info.imageExtent = swapchain_size;
            swapchain_info.imageArrayLayers = 1;
            swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapchain_info.queueFamilyIndexCount = 0;
            swapchain_info.pQueueFamilyIndices = nullptr;
            swapchain_info.preTransform = transform_flag;
            swapchain_info.compositeAlpha = composite_alpha_flag;
            swapchain_info.presentMode = present_mode;
            swapchain_info.clipped = VK_TRUE;
            swapchain_info.oldSwapchain = old_swapchain;

            err = fpCreateSwapchainKHR(m_device, &swapchain_info, nullptr, &m_swapchain);
            assert(!err);

            Log("create swapchain w: %d h: %d, surface w: %d h: %d", swapchain_size.width, swapchain_size.height, surface_caps.currentExtent.width, surface_caps.currentExtent.height);

            if (old_swapchain != VK_NULL_HANDLE)
            {
                fpDestroySwapchainKHR(m_device, old_swapchain, nullptr);
            }

            err = fpGetSwapchainImagesKHR(m_device, m_swapchain, &swapchain_image_count, nullptr);
            assert(!err);
            assert(swapchain_image_count > 0);

            Vector<VkImage> swapchain_images(swapchain_image_count);
            err = fpGetSwapchainImagesKHR(m_device, m_swapchain, &swapchain_image_count, &swapchain_images[0]);
            assert(!err);

            m_swapchain_image_resources.Resize(swapchain_image_count);
            for (int i = 0; i < m_swapchain_image_resources.Size(); ++i)
            {
                SwapchainImageResources& resource = m_swapchain_image_resources[i];

                VkImageViewCreateInfo view_info;
                Memory::Zero(&view_info, sizeof(view_info));
                view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                view_info.pNext = nullptr;
                view_info.flags = 0;
                view_info.image = swapchain_images[i];
                view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
                view_info.format = m_surface_format.format;
                view_info.components = {
                    VK_COMPONENT_SWIZZLE_R,
                    VK_COMPONENT_SWIZZLE_G,
                    VK_COMPONENT_SWIZZLE_B,
                    VK_COMPONENT_SWIZZLE_A,
                };
                view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                view_info.subresourceRange.baseMipLevel = 0;
                view_info.subresourceRange.levelCount = 1;
                view_info.subresourceRange.baseArrayLayer = 0;
                view_info.subresourceRange.layerCount = 1;

                resource.width = swapchain_size.width;
                resource.height = swapchain_size.height;
                resource.format = m_surface_format.format;
                resource.image = swapchain_images[i];

                err = vkCreateImageView(m_device, &view_info, nullptr, &resource.image_view);
                assert(!err);
            }
        }

        void CreateCommandPool(int queue_family_index, VkCommandPool* cmd_pool)
        {
            VkCommandPoolCreateInfo pool_info;
            Memory::Zero(&pool_info, sizeof(pool_info));
            pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            pool_info.pNext = nullptr;
            pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            pool_info.queueFamilyIndex = queue_family_index;

            VkResult err = vkCreateCommandPool(m_device, &pool_info, nullptr, cmd_pool);
            assert(!err);
        }

        void CreateCommandBuffer(VkCommandPool cmd_pool, VkCommandBufferLevel level, VkCommandBuffer* cmd)
        {
            VkCommandBufferAllocateInfo cmd_info;
            Memory::Zero(&cmd_info, sizeof(cmd_info));
            cmd_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            cmd_info.pNext = nullptr;
            cmd_info.commandPool = cmd_pool;
            cmd_info.level = level;
            cmd_info.commandBufferCount = 1;

            VkResult err = vkAllocateCommandBuffers(m_device, &cmd_info, cmd);
            assert(!err);
        }

        void CreateRenderPass(
            VkFormat color_format,
            VkFormat depth_format,
            const Vector<VkFormat>& extra_color_formats,
            CameraClearFlags clear_flag,
            int sample_count,
            bool present,
            bool stereo_rendering,
            VkRenderPass* render_pass)
        {
            VkAttachmentLoadOp color_load;
            VkAttachmentLoadOp depth_load;
			VkImageLayout color_final_layout;
			VkImageLayout depth_final_layout;
			VkImageLayout color_initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
			VkImageLayout depth_initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;

            switch (clear_flag)
            {
                case CameraClearFlags::Color:
                {
                    color_load = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    depth_load = VK_ATTACHMENT_LOAD_OP_LOAD;
                    break;
                }
                case CameraClearFlags::Depth:
                {
                    color_load = VK_ATTACHMENT_LOAD_OP_LOAD;
                    depth_load = VK_ATTACHMENT_LOAD_OP_CLEAR;
					color_initial_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    break;
                }
                case CameraClearFlags::ColorAndDepth:
                {
                    color_load = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    depth_load = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    break;
                }
                case CameraClearFlags::Nothing:
                {
					color_load = VK_ATTACHMENT_LOAD_OP_LOAD;
					depth_load = VK_ATTACHMENT_LOAD_OP_LOAD;
					color_initial_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					depth_initial_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    break;
                }
                default:
                {
					color_load = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
					depth_load = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    break;
                }
            }

            if (present)
            {
                color_final_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                depth_final_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            }
            else
            {
                color_final_layout = VK_IMAGE_LAYOUT_GENERAL;
                depth_final_layout = VK_IMAGE_LAYOUT_GENERAL;
            }

            Vector<VkAttachmentReference> color_references;
            VkAttachmentReference depth_reference = {
                0,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            };

            Vector<VkAttachmentDescription> attachments;

            if (color_format != VK_FORMAT_UNDEFINED)
            {
                VkAttachmentReference color_reference;
                color_reference.attachment = attachments.Size();
                color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                color_references.Add(color_reference);

                VkAttachmentDescription attachment;
                Memory::Zero(&attachment, sizeof(attachment));
                attachment.flags = 0;
                attachment.format = color_format;
                attachment.samples = (VkSampleCountFlagBits) sample_count;
                attachment.loadOp = color_load;
                attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attachment.initialLayout = color_initial_layout;
                attachment.finalLayout = color_final_layout;

                attachments.Add(attachment);
            }

            for (int i = 0; i < extra_color_formats.Size(); ++i)
            {
                VkAttachmentReference color_reference;
                color_reference.attachment = attachments.Size();
                color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                color_references.Add(color_reference);

                VkAttachmentDescription attachment;
                Memory::Zero(&attachment, sizeof(attachment));
                attachment.flags = 0;
                attachment.format = extra_color_formats[i];
                attachment.samples = (VkSampleCountFlagBits) sample_count;
                attachment.loadOp = color_load;
                attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attachment.initialLayout = color_initial_layout;
                attachment.finalLayout = color_final_layout;

                attachments.Add(attachment);
            }

            if (depth_format != VK_FORMAT_UNDEFINED)
            {
                depth_reference.attachment = attachments.Size();

                VkAttachmentDescription attachment;
                Memory::Zero(&attachment, sizeof(attachment));
                attachment.flags = 0;
                attachment.format = depth_format;
                attachment.samples = (VkSampleCountFlagBits) sample_count;
                attachment.loadOp = depth_load;
                attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attachment.initialLayout = depth_initial_layout;
                attachment.finalLayout = depth_final_layout;

                attachments.Add(attachment);
            }

            VkSubpassDescription subpass;
            Memory::Zero(&subpass, sizeof(subpass));
            subpass.flags = 0;
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.inputAttachmentCount = 0;
            subpass.pInputAttachments = nullptr;
            if (color_format != VK_FORMAT_UNDEFINED)
            {
                subpass.colorAttachmentCount = color_references.Size();
                subpass.pColorAttachments = &color_references[0];
            }
            else
            {
                subpass.colorAttachmentCount = 0;
                subpass.pColorAttachments = nullptr;
            }
            subpass.pResolveAttachments = nullptr;
            if (depth_format != VK_FORMAT_UNDEFINED)
            {
                subpass.pDepthStencilAttachment = &depth_reference;
            }
            else
            {
                subpass.pDepthStencilAttachment = nullptr;
            }
            subpass.preserveAttachmentCount = 0;
            subpass.pPreserveAttachments = nullptr;

            VkSubpassDependency dependency;
            Memory::Zero(&dependency, sizeof(dependency));
            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass = 0;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            dependency.dependencyFlags = 0;

            VkRenderPassCreateInfo rp_info;
            Memory::Zero(&rp_info, sizeof(rp_info));
            rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            rp_info.pNext = nullptr;
            rp_info.flags = 0;
            rp_info.attachmentCount = attachments.Size();
            rp_info.pAttachments = &attachments[0];
            rp_info.subpassCount = 1;
            rp_info.pSubpasses = &subpass;
            rp_info.dependencyCount = 1;
            rp_info.pDependencies = &dependency;

            const uint32_t view_mask = 0b00000011;
            const uint32_t correlation_mask = 0b00000011;
            VkRenderPassMultiviewCreateInfo rpm_info;
            if (stereo_rendering)
            {
                Memory::Zero(&rpm_info, sizeof(rpm_info));
                rpm_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO;
                rpm_info.pNext = nullptr;
                rpm_info.subpassCount = 1;
                rpm_info.pViewMasks = &view_mask;
                rpm_info.dependencyCount = 0;
                rpm_info.pViewOffsets = nullptr;
                rpm_info.correlationMaskCount = 1;
                rpm_info.pCorrelationMasks = &correlation_mask;

                rp_info.pNext = &rpm_info;
            }

            VkResult err = vkCreateRenderPass(m_device, &rp_info, nullptr, render_pass);
            assert(!err);
        }

        void CreateFramebuffer(
            VkImageView color_image_view,
            VkImageView depth_image_view,
            const Vector<VkImageView>& extra_color_image_views,
            int image_width,
            int image_height,
            VkRenderPass render_pass,
            VkFramebuffer* framebuffer)
        {
            Vector<VkImageView> attachments_view;
            if (color_image_view != VK_NULL_HANDLE)
            {
                attachments_view.Add(color_image_view);
            }
            for (int i = 0; i < extra_color_image_views.Size(); ++i)
            {
                attachments_view.Add(extra_color_image_views[i]);
            }
            if (depth_image_view != VK_NULL_HANDLE)
            {
                attachments_view.Add(depth_image_view);
            }

            VkFramebufferCreateInfo fb_info;
            Memory::Zero(&fb_info, sizeof(fb_info));
            fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fb_info.pNext = nullptr;
            fb_info.flags = 0;
            fb_info.renderPass = render_pass;
            fb_info.attachmentCount = attachments_view.Size();
            fb_info.pAttachments = &attachments_view[0];
            fb_info.width = (uint32_t) image_width;
            fb_info.height = (uint32_t) image_height;
            fb_info.layers = 1;

            VkResult err = vkCreateFramebuffer(m_device, &fb_info, nullptr, framebuffer);
            assert(!err);
        }

        bool CheckMemoryType(uint32_t mem_type_bits, VkFlags requirements_mask, uint32_t* type_index)
        {
            for (int i = 0; i < VK_MAX_MEMORY_TYPES; ++i)
            {
                if ((mem_type_bits & 1) == 1)
                {
                    if ((m_memory_properties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask)
                    {
                        *type_index = i;
                        return true;
                    }
                }
                mem_type_bits >>= 1;
            }

            return false;
        }

        VkFormat ChooseFormatSupported(const Vector<VkFormat>& formats, VkFormatFeatureFlags features)
        {
            for (int i = 0; i < formats.Size(); ++i)
            {
                VkFormatProperties properties;
                vkGetPhysicalDeviceFormatProperties(m_gpu, formats[i], &properties);

                if ((properties.optimalTilingFeatures & features) == features)
                {
                    return formats[i];
                }
            }

            return VK_FORMAT_UNDEFINED;
        }

        int GetMaxSamples()
        {
            VkSampleCountFlags counts = Mathf::Min(m_gpu_properties.limits.framebufferColorSampleCounts, m_gpu_properties.limits.framebufferDepthSampleCounts);
            if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
            if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
            if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
            if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
            if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
            if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }
            return VK_SAMPLE_COUNT_1_BIT;
        }

        Ref<Texture> CreateTexture(
            VkImageType type,
            VkImageViewType view_type,
            int width,
            int height,
            VkFormat format,
            VkImageUsageFlags usage,
            VkImageAspectFlags aspect_flag,
            const VkComponentMapping& component,
            int mipmap_level_count,
            bool cubemap,
            int array_size,
            int sample_count)
        {
            Ref<Texture> texture = Ref<Texture>(new Texture());
            texture->m_width = width;
            texture->m_height = height;
            texture->m_format = format;
            texture->m_mipmap_level_count = mipmap_level_count;
            texture->m_cubemap = cubemap;
            texture->m_array_size = array_size;
            texture->m_sample_count = sample_count;

            VkImageCreateFlags flags = 0;
            uint32_t layer_count = 1;

            if (cubemap)
            {
                flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
                layer_count = 6;
            }
            if (array_size > 1)
            {
                layer_count = array_size;
            }

            VkImageCreateInfo image_info;
            Memory::Zero(&image_info, sizeof(image_info));
            image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image_info.pNext = nullptr;
            image_info.flags = flags;
            image_info.imageType = type;
            image_info.format = format;
            image_info.extent = { (uint32_t) width, (uint32_t) height, 1 };
            image_info.mipLevels = mipmap_level_count;
            image_info.arrayLayers = layer_count;
            image_info.samples = VK_SAMPLE_COUNT_1_BIT;
            image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
            if (sample_count > 1)
            {
                image_info.usage = usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            }
            else
            {
                image_info.usage = usage;
            }
            image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            image_info.queueFamilyIndexCount = 0;
            image_info.pQueueFamilyIndices = nullptr;
            image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            VkResult err = vkCreateImage(m_device, &image_info, nullptr, &texture->m_image);
            assert(!err);

            if (sample_count > 1)
            {
                image_info.samples = (VkSampleCountFlagBits) sample_count;
                image_info.usage = usage | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
                err = vkCreateImage(m_device, &image_info, nullptr, &texture->m_image_multi_sample);
                assert(!err);
            }

            VkMemoryRequirements mem_reqs;
            vkGetImageMemoryRequirements(m_device, texture->m_image, &mem_reqs);

            Memory::Zero(&texture->m_memory_info, sizeof(texture->m_memory_info));
            texture->m_memory_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            texture->m_memory_info.pNext = nullptr;
            texture->m_memory_info.allocationSize = mem_reqs.size;
            texture->m_memory_info.memoryTypeIndex = 0;

            bool pass = this->CheckMemoryType(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture->m_memory_info.memoryTypeIndex);
            assert(pass);

            err = vkAllocateMemory(m_device, &texture->m_memory_info, nullptr, &texture->m_memory);
            assert(!err);

            err = vkBindImageMemory(m_device, texture->m_image, texture->m_memory, 0);
            assert(!err);

            if (sample_count > 1)
            {
                vkGetImageMemoryRequirements(m_device, texture->m_image_multi_sample, &mem_reqs);

                Memory::Zero(&texture->m_memory_info_multi_sample, sizeof(texture->m_memory_info_multi_sample));
                texture->m_memory_info_multi_sample.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                texture->m_memory_info_multi_sample.pNext = nullptr;
                texture->m_memory_info_multi_sample.allocationSize = mem_reqs.size;
                texture->m_memory_info_multi_sample.memoryTypeIndex = 0;

                pass = this->CheckMemoryType(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture->m_memory_info_multi_sample.memoryTypeIndex);
                assert(pass);

                err = vkAllocateMemory(m_device, &texture->m_memory_info_multi_sample, nullptr, &texture->m_memory_multi_sample);
                assert(!err);

                err = vkBindImageMemory(m_device, texture->m_image_multi_sample, texture->m_memory_multi_sample, 0);
                assert(!err);
            }

            VkImageViewCreateInfo view_info;
            Memory::Zero(&view_info, sizeof(view_info));
            view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            view_info.pNext = nullptr;
            view_info.flags = 0;
            view_info.image = texture->m_image;
            view_info.viewType = view_type;
            view_info.format = format;
            view_info.components = component;
            view_info.subresourceRange = { aspect_flag, 0, (uint32_t) mipmap_level_count, 0, layer_count };
            
            err = vkCreateImageView(m_device, &view_info, nullptr, &texture->m_image_view);
            assert(!err);

            if (sample_count > 1)
            {
                view_info.image = texture->m_image_multi_sample;
                err = vkCreateImageView(m_device, &view_info, nullptr, &texture->m_image_view_multi_sample);
                assert(!err);
            }

            return texture;
        }

        void CreateSampler(
            const Ref<Texture>& texture,
            VkFilter filter_mode,
            VkSamplerAddressMode wrap_mode)
        {
            VkSamplerCreateInfo sampler_info;
            Memory::Zero(&sampler_info, sizeof(sampler_info));
            sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            sampler_info.pNext = nullptr;
            sampler_info.flags = 0;
            sampler_info.magFilter = filter_mode;
            sampler_info.minFilter = filter_mode;
            sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            sampler_info.addressModeU = wrap_mode;
            sampler_info.addressModeV = wrap_mode;
            sampler_info.addressModeW = wrap_mode;
            sampler_info.mipLodBias = 0.0f;
            sampler_info.anisotropyEnable = VK_FALSE;
            sampler_info.maxAnisotropy = 1;
            sampler_info.compareEnable = VK_FALSE;
            sampler_info.compareOp = VK_COMPARE_OP_NEVER;
            sampler_info.minLod = 0.0f;
            sampler_info.maxLod = texture->m_mipmap_level_count > 1 ? (float) texture->m_mipmap_level_count : 0.0f;
            sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
            sampler_info.unnormalizedCoordinates = VK_FALSE;

            VkResult err = vkCreateSampler(m_device, &sampler_info, nullptr, &texture->m_sampler);
            assert(!err);
        }

        Ref<BufferObject> CreateBuffer(const void* data, int size, VkBufferUsageFlags usage)
        {
            Ref<BufferObject> buffer = RefMake<BufferObject>(size);

            VkBufferCreateInfo buffer_info;
            Memory::Zero(&buffer_info, sizeof(buffer_info));
            buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            buffer_info.pNext = nullptr;
            buffer_info.flags = 0;
            buffer_info.size = (VkDeviceSize) size;
            buffer_info.usage = usage;
            buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            buffer_info.queueFamilyIndexCount = 0;
            buffer_info.pQueueFamilyIndices = nullptr;

            VkResult err = vkCreateBuffer(m_device, &buffer_info, nullptr, &buffer->m_buffer);
            assert(!err);

            VkMemoryRequirements mem_reqs;
            vkGetBufferMemoryRequirements(m_device, buffer->m_buffer, &mem_reqs);

            Memory::Zero(&buffer->m_memory_info, sizeof(buffer->m_memory_info));
            buffer->m_memory_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            buffer->m_memory_info.pNext = nullptr;
            buffer->m_memory_info.allocationSize = mem_reqs.size;
            buffer->m_memory_info.memoryTypeIndex = 0;

            bool pass = this->CheckMemoryType(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer->m_memory_info.memoryTypeIndex);
            assert(pass);

            err = vkAllocateMemory(m_device, &buffer->m_memory_info, nullptr, &buffer->m_memory);
            assert(!err);

            err = vkBindBufferMemory(m_device, buffer->m_buffer, buffer->m_memory, 0);
            assert(!err);

            if (data)
            {
                void* map_data = nullptr;
                err = vkMapMemory(m_device, buffer->m_memory, 0, size, 0, (void**) &map_data);
                assert(!err);

                Memory::Copy(map_data, data, size);

                vkUnmapMemory(m_device, buffer->m_memory);
            }

            return buffer;
        }

        void UpdateBuffer(const Ref<BufferObject>& buffer, int buffer_offset, const void* data, int size)
        {
            void* map_data = nullptr;
            VkResult err = vkMapMemory(m_device, buffer->GetMemory(), buffer_offset, size, 0, (void**) &map_data);
            assert(!err);

            Memory::Copy(map_data, data, size);

            vkUnmapMemory(m_device, buffer->GetMemory());
        }

        void ReadBuffer(const Ref<BufferObject>& buffer, ByteBuffer& data)
        {
            if (data.Size() < buffer->GetSize())
            {
                return;
            }

            void* map_data = nullptr;
            VkResult err = vkMapMemory(m_device, buffer->GetMemory(), 0, buffer->GetSize(), 0, (void**) &map_data);
            assert(!err);

            Memory::Copy(&data[0], map_data, buffer->GetSize());

            vkUnmapMemory(m_device, buffer->GetMemory());
        }

        void BeginImageCmd()
        {
            m_image_cmd_mutex.lock();

            VkCommandBufferBeginInfo cmd_info;
            cmd_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            cmd_info.pNext = nullptr;
            cmd_info.flags = 0;
            cmd_info.pInheritanceInfo = nullptr;

            VkResult err = vkBeginCommandBuffer(m_image_cmd, &cmd_info);
            assert(!err);
        }

        void EndImageCmd()
        {
            VkResult err = vkEndCommandBuffer(m_image_cmd);
            assert(!err);

            VkSubmitInfo submit_info;
            Memory::Zero(&submit_info, sizeof(submit_info));
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.pNext = nullptr;
            submit_info.waitSemaphoreCount = 0;
            submit_info.pWaitSemaphores = nullptr;
            submit_info.pWaitDstStageMask = nullptr;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &m_image_cmd;
            submit_info.signalSemaphoreCount = 0;
            submit_info.pSignalSemaphores = nullptr;

            err = vkResetFences(m_device, 1, &m_image_fence);
            assert(!err);

            err = vkQueueSubmit(m_graphics_queue, 1, &submit_info, m_image_fence);
            assert(!err);

            err = vkWaitForFences(m_device, 1, &m_image_fence, VK_TRUE, UINT64_MAX);
            assert(!err);

            m_image_cmd_mutex.unlock();
        }

        void SetImageLayout(
			VkCommandBuffer cmd,
            VkImage image,
			VkPipelineStageFlags src_stage,
			VkPipelineStageFlags dst_stage,
            const VkImageSubresourceRange& subresource_range,
            VkImageLayout old_image_layout,
            VkImageLayout new_image_layout,
            VkAccessFlagBits src_access_mask)
        {
            VkImageMemoryBarrier barrier_info;
            Memory::Zero(&barrier_info, sizeof(barrier_info));
            barrier_info.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier_info.pNext = NULL;
            barrier_info.srcAccessMask = src_access_mask;
            barrier_info.dstAccessMask = 0;
            barrier_info.oldLayout = old_image_layout;
            barrier_info.newLayout = new_image_layout;
            barrier_info.srcQueueFamilyIndex = m_graphics_queue_family_index;
            barrier_info.dstQueueFamilyIndex = m_graphics_queue_family_index;
            barrier_info.image = image;
            barrier_info.subresourceRange = subresource_range;

            switch (new_image_layout)
            {
                case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                    barrier_info.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    break;

                case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                    barrier_info.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    break;

                case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                    barrier_info.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                    break;

                case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                    barrier_info.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
                    break;

                case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                    barrier_info.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                    break;

                case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
                    barrier_info.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                    break;

                default:
                    barrier_info.dstAccessMask = 0;
                    break;
            }

            vkCmdPipelineBarrier(cmd, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier_info);
        }

        void CreateSpirvShaderModule(const Vector<unsigned int>& spirv, VkShaderModule* module)
        {
            VkShaderModuleCreateInfo create_info;
            Memory::Zero(&create_info, sizeof(create_info));
            create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            create_info.pNext = nullptr;
            create_info.flags = 0;
            create_info.codeSize = spirv.SizeInBytes();
            create_info.pCode = &spirv[0];

            VkResult err = vkCreateShaderModule(m_device, &create_info, nullptr, module);
            assert(!err);
        }

        void CreateGlslShaderModule(
            const String& glsl,
            VkShaderStageFlagBits shader_type,
            VkShaderModule* module,
            Vector<VertexAttribute>& attributes,
            Vector<UniformSet>& uniform_sets)
        {
            Vector<unsigned int> spirv;
            GlslToSpirvCached(glsl, shader_type, spirv);
            this->CreateSpirvShaderModule(spirv, module);

            // reflect spirv
            spirv_cross::CompilerGLSL compiler(&spirv[0], spirv.Size());
            spirv_cross::ShaderResources resources = compiler.get_shader_resources();

            if (shader_type == VK_SHADER_STAGE_VERTEX_BIT)
            {
                for (const auto& resource : resources.stage_inputs)
                {
                    uint32_t location = compiler.get_decoration(resource.id, spv::DecorationLocation);
                    const std::string& name = compiler.get_name(resource.id);
                    const auto& type = compiler.get_type(resource.type_id);

                    assert(type.basetype == spirv_cross::SPIRType::Float);
                    assert(type.array.size() == 0);

                    VertexAttribute attr;
                    attr.location = location;
                    attr.name = name.c_str();
                    attr.vector_size = type.vecsize;

                    attributes.Add(attr);
                }
            }

            for (const auto& resource : resources.uniform_buffers)
            {
                uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
                uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
                const std::string& name = compiler.get_name(resource.id);

                UniformSet* set_ptr = nullptr;
                for (int i = 0; i < uniform_sets.Size(); ++i)
                {
                    if (set == uniform_sets[i].set)
                    {
                        set_ptr = &uniform_sets[i];
                        break;
                    }
                }
                if (set_ptr == nullptr)
                {
                    uniform_sets.Add(UniformSet());
                    set_ptr = &uniform_sets[uniform_sets.Size() - 1];
                    set_ptr->set = set;
                }

                UniformBuffer buffer;
                buffer.name = name.c_str();
                buffer.binding = (int) binding;
                buffer.stage = shader_type;

                const spirv_cross::SPIRType& type = compiler.get_type(resource.base_type_id);

                int max_offset_member = -1;
                int max_offset = -1;
                for (size_t i = 0; i < type.member_types.size(); ++i)
                {
                    const std::string& member_name = compiler.get_member_name(type.self, (uint32_t) i);
                    int member_offset = (int) compiler.type_struct_member_offset(type, (uint32_t) i);
                    int member_size = (int) compiler.get_declared_struct_member_size(type, (uint32_t) i);

                    UniformMember member;
                    member.name = member_name.c_str();
                    member.offset = member_offset;
                    member.size = member_size;

                    buffer.members.Add(member);

                    if (member.offset > max_offset)
                    {
                        max_offset = member.offset;
                        max_offset_member = (int) i;
                    }
                }

                buffer.size = buffer.members[max_offset_member].offset + buffer.members[max_offset_member].size;

                set_ptr->buffers.Add(buffer);
            }

            for (const auto& resource : resources.sampled_images)
            {
                uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
                uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
                const std::string& name = resource.name;

                UniformSet* set_ptr = nullptr;
                for (int i = 0; i < uniform_sets.Size(); ++i)
                {
                    if (set == uniform_sets[i].set)
                    {
                        set_ptr = &uniform_sets[i];
                        break;
                    }
                }
                if (set_ptr == nullptr)
                {
                    uniform_sets.Add(UniformSet());
                    set_ptr = &uniform_sets[uniform_sets.Size() - 1];
                    set_ptr->set = set;
                }

                UniformTexture texture;
                texture.name = name.c_str();
                texture.binding = (int) binding;
                texture.stage = shader_type;

                set_ptr->textures.Add(texture);
            }

            for (const auto& resource : resources.storage_images)
            {
                uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
                uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
                const std::string& name = resource.name;

                UniformSet* set_ptr = nullptr;
                for (int i = 0; i < uniform_sets.Size(); ++i)
                {
                    if (set == uniform_sets[i].set)
                    {
                        set_ptr = &uniform_sets[i];
                        break;
                    }
                }
                if (set_ptr == nullptr)
                {
                    uniform_sets.Add(UniformSet());
                    set_ptr = &uniform_sets[uniform_sets.Size() - 1];
                    set_ptr->set = set;
                }

                UniformTexture texture;
                texture.name = name.c_str();
                texture.binding = (int) binding;
                texture.stage = shader_type;

                set_ptr->textures.Add(texture);
            }

            for (const auto& resource : resources.storage_buffers)
            {
                uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
                uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
                const std::string& name = resource.name;

                UniformSet* set_ptr = nullptr;
                for (int i = 0; i < uniform_sets.Size(); ++i)
                {
                    if (set == uniform_sets[i].set)
                    {
                        set_ptr = &uniform_sets[i];
                        break;
                    }
                }
                if (set_ptr == nullptr)
                {
                    uniform_sets.Add(UniformSet());
                    set_ptr = &uniform_sets[uniform_sets.Size() - 1];
                    set_ptr->set = set;
                }

                StorageBuffer buffer;
                buffer.name = name.c_str();
                buffer.binding = (int) binding;
                buffer.stage = shader_type;

                set_ptr->storage_buffers.Add(buffer);
            }
        }

        void CreatePipelineCache(VkPipelineCache* pipeline_cache)
        {
            VkPipelineCacheCreateInfo create_info;
            Memory::Zero(&create_info, sizeof(create_info));
            create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
            create_info.pNext = nullptr;
            create_info.flags = 0;
            create_info.initialDataSize = 0;
            create_info.pInitialData = nullptr;

            VkResult err;
            err = vkCreatePipelineCache(m_device, &create_info, nullptr, pipeline_cache);
            assert(!err);
        }

        void CreateShaderModule(
            const String& vs_predefine,
            const Vector<String>& vs_includes,
            const String& vs_source,
            const String& fs_predefine,
            const Vector<String>& fs_includes,
            const String& fs_source,
            VkShaderModule* vs_module,
            VkShaderModule* fs_module,
            Vector<VertexAttribute>& attributes,
            Vector<UniformSet>& uniform_sets)
        {
            Vector<String> includes;
            includes.Add("Base.in");
            if (vs_includes.Size() > 0)
            {
                includes.AddRange(&vs_includes[0], vs_includes.Size());
            }
            String vs = ProcessShaderSource(vs_source, vs_predefine, includes);
            String fs = ProcessShaderSource(fs_source, fs_predefine, fs_includes);

            this->CreateGlslShaderModule(vs, VK_SHADER_STAGE_VERTEX_BIT, vs_module, attributes, uniform_sets);
            this->CreateGlslShaderModule(fs, VK_SHADER_STAGE_FRAGMENT_BIT, fs_module, attributes, uniform_sets);

            // sort by set
            List<UniformSet*> sets;
            for (int i = 0; i < uniform_sets.Size(); ++i)
            {
                sets.AddLast(&uniform_sets[i]);
            }
            sets.Sort([](const UniformSet* a, const UniformSet* b) {
                return a->set < b->set;
            });

            Vector<UniformSet> sets_sorted;
            for (auto i : sets)
            {
                sets_sorted.Add(*i);
            }
            uniform_sets = sets_sorted;
        }

        void CreateComputeShaderModule(
            const String& cs_source,
            VkShaderModule* cs_module,
            Vector<UniformSet>& uniform_sets)
        {
            Vector<VertexAttribute> attributes;
            this->CreateGlslShaderModule(cs_source, VK_SHADER_STAGE_COMPUTE_BIT, cs_module, attributes, uniform_sets);
        }

        void CreatePipelineLayout(
            const Vector<UniformSet>& uniform_sets,
            Vector<VkDescriptorSetLayout>& descriptor_layouts,
            VkPipelineLayout* pipeline_layout)
        {
            VkResult err;

            descriptor_layouts.Resize(uniform_sets.Size());
            for (int i = 0; i < uniform_sets.Size(); ++i)
            {
                Vector<VkDescriptorSetLayoutBinding> layout_bindings;

                for (int j = 0; j < uniform_sets[i].buffers.Size(); ++j)
                {
                    const auto& buffer = uniform_sets[i].buffers[j];

                    VkDescriptorSetLayoutBinding layout_binding;
                    Memory::Zero(&layout_binding, sizeof(layout_binding));
                    layout_binding.binding = buffer.binding;
                    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    layout_binding.descriptorCount = 1;
                    layout_binding.stageFlags = buffer.stage;
                    layout_binding.pImmutableSamplers = nullptr;

                    layout_bindings.Add(layout_binding);
                }

                for (int j = 0; j < uniform_sets[i].textures.Size(); ++j)
                {
                    const auto& texture = uniform_sets[i].textures[j];

                    VkDescriptorSetLayoutBinding layout_binding;
                    Memory::Zero(&layout_binding, sizeof(layout_binding));
                    layout_binding.binding = texture.binding;
                    if (texture.stage == VK_SHADER_STAGE_COMPUTE_BIT)
                    {
                        layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    }
                    else
                    {
                        layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    }
                    layout_binding.descriptorCount = 1;
                    layout_binding.stageFlags = texture.stage;
                    layout_binding.pImmutableSamplers = nullptr;

                    layout_bindings.Add(layout_binding);
                }

                for (int j = 0; j < uniform_sets[i].storage_buffers.Size(); ++j)
                {
                    const auto& buffer = uniform_sets[i].storage_buffers[j];

                    VkDescriptorSetLayoutBinding layout_binding;
                    Memory::Zero(&layout_binding, sizeof(layout_binding));
                    layout_binding.binding = buffer.binding;
                    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    layout_binding.descriptorCount = 1;
                    layout_binding.stageFlags = buffer.stage;
                    layout_binding.pImmutableSamplers = nullptr;

                    layout_bindings.Add(layout_binding);
                }

                VkDescriptorSetLayoutCreateInfo descriptor_layout;
                Memory::Zero(&descriptor_layout, sizeof(descriptor_layout));
                descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                descriptor_layout.pNext = nullptr;
                descriptor_layout.flags = 0;
                descriptor_layout.bindingCount = layout_bindings.Size();
                descriptor_layout.pBindings = &layout_bindings[0];

                err = vkCreateDescriptorSetLayout(m_device, &descriptor_layout, nullptr, &descriptor_layouts[i]);
                assert(!err);
            }

            VkPipelineLayoutCreateInfo pipeline_layout_info;
            Memory::Zero(&pipeline_layout_info, sizeof(pipeline_layout_info));
            pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_info.pNext = nullptr;
            pipeline_layout_info.flags = 0;
            pipeline_layout_info.setLayoutCount = descriptor_layouts.Size();
            if (descriptor_layouts.Size() > 0)
            {
                pipeline_layout_info.pSetLayouts = &descriptor_layouts[0];
            }
            else
            {
                pipeline_layout_info.pSetLayouts = nullptr;
            }
            pipeline_layout_info.pushConstantRangeCount = 0;
            pipeline_layout_info.pPushConstantRanges = nullptr;

            err = vkCreatePipelineLayout(m_device, &pipeline_layout_info, nullptr, pipeline_layout);
            assert(!err);
        }

        void CreatePipeline(
            VkRenderPass render_pass,
            const Vector<VertexAttribute>& attributes,
            VkShaderModule vs_module,
            VkShaderModule fs_module,
            const RenderState& render_state,
            VkPipelineLayout pipeline_layout,
            VkPipelineCache pipeline_cache,
            VkPipeline* pipeline,
            bool color_attachment,
            bool depth_attachment,
            int extra_color_attachment_count,
            int sample_count,
            bool instancing,
            int instance_stride)
        {
            Vector<VkPipelineShaderStageCreateInfo> shader_stages;
            {
                VkPipelineShaderStageCreateInfo stage_info;
                Memory::Zero(&stage_info, sizeof(stage_info));
                stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                stage_info.pNext = nullptr;
                stage_info.flags = 0;
                stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
                stage_info.module = vs_module;
                stage_info.pName = "main";
                stage_info.pSpecializationInfo = nullptr;

                shader_stages.Add(stage_info);
            }

            if (color_attachment)
            {
                VkPipelineShaderStageCreateInfo stage_info;
                Memory::Zero(&stage_info, sizeof(stage_info));
                stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                stage_info.pNext = nullptr;
                stage_info.flags = 0;
                stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                stage_info.module = fs_module;
                stage_info.pName = "main";
                stage_info.pSpecializationInfo = nullptr;

                shader_stages.Add(stage_info);
            }

            Vector<VkVertexInputBindingDescription> vi_binds;
            VkVertexInputBindingDescription vi_bind;
            Memory::Zero(&vi_bind, sizeof(vi_bind));
            vi_bind.binding = VERTEX_INPUT_BINDING_VERTEX;
            vi_bind.stride = sizeof(Vertex);
            vi_bind.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            vi_binds.Add(vi_bind);

            Vector<VkVertexInputAttributeDescription> vi_attrs;
            for (int i = 0; i < attributes.Size(); ++i)
            {
                int location = attributes[i].location;

                if (location < (int) VertexAttributeType::Count)
                {
                    VkVertexInputAttributeDescription attr;
                    attr.location = location;
                    attr.binding = VERTEX_INPUT_BINDING_VERTEX;
                    switch (attributes[i].vector_size)
                    {
                    case 2:
                        attr.format = VK_FORMAT_R32G32_SFLOAT;
                        break;
                    case 3:
                        attr.format = VK_FORMAT_R32G32B32_SFLOAT;
                        break;
                    case 4:
                        attr.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                        break;
                    default:
                        assert(!"invalid vertex attribute vector size");
                        break;
                    }
                    attr.offset = VERTEX_ATTR_OFFSETS[location];

                    vi_attrs.Add(attr);
                }
                else if (instancing && location >= (int) InstanceVertexAttributeLocation::TransformMatrixRow0 && attributes[i].name.StartsWith("a_instance_"))
                {
                    VkVertexInputAttributeDescription attr;
                    attr.location = location;
                    attr.binding = VERTEX_INPUT_BINDING_INSTANCE;
                    attr.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                    attr.offset = sizeof(float) * 4 * (location - (int) InstanceVertexAttributeLocation::TransformMatrixRow0);

                    vi_attrs.Add(attr);
                }
                else
                {
                    assert(!"invalid vertex attribute");
                }
            }

            if (instancing)
            {
                Memory::Zero(&vi_bind, sizeof(vi_bind));
                vi_bind.binding = VERTEX_INPUT_BINDING_INSTANCE;
                vi_bind.stride = instance_stride;
                vi_bind.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
                vi_binds.Add(vi_bind);
            }

            VkPipelineVertexInputStateCreateInfo vi;
            Memory::Zero(&vi, sizeof(vi));
            vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vi.pNext = nullptr;
            vi.flags = 0;
            vi.vertexBindingDescriptionCount = (uint32_t) vi_binds.Size();
            vi.pVertexBindingDescriptions = &vi_binds[0];
            vi.vertexAttributeDescriptionCount = (uint32_t) vi_attrs.Size();
            vi.pVertexAttributeDescriptions = &vi_attrs[0];

            VkPipelineInputAssemblyStateCreateInfo ia;
            Memory::Zero(&ia, sizeof(ia));
            ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            ia.pNext = nullptr;
            ia.flags = 0;
            ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            ia.primitiveRestartEnable = VK_FALSE;

            VkPipelineViewportStateCreateInfo vp;
            Memory::Zero(&vp, sizeof(vp));
            vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            vp.pNext = nullptr;
            vp.flags = 0;
            vp.viewportCount = 1;
            vp.pViewports = nullptr;
            vp.scissorCount = 1;
            vp.pScissors = nullptr;

            Vector<VkDynamicState> dynamic_states;
            dynamic_states.Add(VK_DYNAMIC_STATE_VIEWPORT);
            dynamic_states.Add(VK_DYNAMIC_STATE_SCISSOR);

            VkPipelineDynamicStateCreateInfo dynamic_info;
            Memory::Zero(&dynamic_info, sizeof(dynamic_info));
            dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamic_info.pNext = nullptr;
            dynamic_info.flags = 0;
            dynamic_info.dynamicStateCount = (uint32_t) dynamic_states.Size();
            dynamic_info.pDynamicStates = &dynamic_states[0];

            VkPipelineRasterizationStateCreateInfo rs;
            Memory::Zero(&rs, sizeof(rs));
            rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rs.pNext = nullptr;
            rs.flags = 0;
            rs.depthClampEnable = VK_FALSE;
            rs.rasterizerDiscardEnable = VK_FALSE;
            rs.polygonMode = VK_POLYGON_MODE_FILL;
            rs.cullMode = (VkCullModeFlags) render_state.cull;
            rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            rs.depthBiasEnable = VK_FALSE;
            rs.depthBiasConstantFactor = 0;
            rs.depthBiasClamp = 0;
            rs.depthBiasSlopeFactor = 0;
            rs.lineWidth = 1.0f;

            VkPipelineMultisampleStateCreateInfo ms;
            Memory::Zero(&ms, sizeof(ms));
            ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            ms.pNext = nullptr;
            ms.flags = 0;
            ms.rasterizationSamples = (VkSampleCountFlagBits) sample_count;
            ms.sampleShadingEnable = VK_FALSE;
            ms.minSampleShading = 0;
            ms.pSampleMask = nullptr;
            ms.alphaToCoverageEnable = VK_FALSE;
            ms.alphaToOneEnable = VK_FALSE;

            VkPipelineDepthStencilStateCreateInfo ds;
            Memory::Zero(&ds, sizeof(ds));
            ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            ds.pNext = nullptr;
            ds.flags = 0;
            ds.depthTestEnable = render_state.zTest != RenderState::ZTest::Off;
            ds.depthWriteEnable = (VkBool32) render_state.zWrite;
            ds.depthCompareOp = render_state.zTest == RenderState::ZTest::Off ? VK_COMPARE_OP_ALWAYS : (VkCompareOp) render_state.zTest;
            ds.depthBoundsTestEnable = VK_FALSE;
            ds.stencilTestEnable = VK_FALSE;
            ds.front = { VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_NEVER, 0, 0, 0 };
            ds.back = ds.front;
            ds.minDepthBounds = 0;
            ds.maxDepthBounds = 0;

            Vector<VkPipelineColorBlendAttachmentState> blend_states;
            VkPipelineColorBlendStateCreateInfo cb;

            if (color_attachment)
            {
                blend_states.Resize(1 + extra_color_attachment_count);
                for (int i = 0; i < blend_states.Size(); ++i)
                {
                    blend_states[i].blendEnable = (VkBool32) render_state.blend;
                    blend_states[i].srcColorBlendFactor = (VkBlendFactor) render_state.srcBlendMode;
                    blend_states[i].dstColorBlendFactor = (VkBlendFactor) render_state.dstBlendMode;
                    blend_states[i].colorBlendOp = VK_BLEND_OP_ADD;
                    blend_states[i].srcAlphaBlendFactor = (VkBlendFactor) render_state.srcBlendMode;
                    blend_states[i].dstAlphaBlendFactor = (VkBlendFactor) render_state.dstBlendMode;
                    blend_states[i].alphaBlendOp = VK_BLEND_OP_ADD;
                    blend_states[i].colorWriteMask =
                        VK_COLOR_COMPONENT_R_BIT |
                        VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT |
                        VK_COLOR_COMPONENT_A_BIT;
                }

                Memory::Zero(&cb, sizeof(cb));
                cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                cb.pNext = nullptr;
                cb.flags = 0;
                cb.logicOpEnable = VK_FALSE;
                cb.logicOp = VK_LOGIC_OP_CLEAR;
                cb.attachmentCount = (uint32_t) blend_states.Size();
                if (blend_states.Size() > 0)
                {
                    cb.pAttachments = &blend_states[0];
                }
                else
                {
                    cb.pAttachments = nullptr;
                }
                cb.blendConstants[0] = 0;
                cb.blendConstants[1] = 0;
                cb.blendConstants[2] = 0;
                cb.blendConstants[3] = 0;
            }

            VkGraphicsPipelineCreateInfo pipeline_info;
            Memory::Zero(&pipeline_info, sizeof(pipeline_info));
            pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipeline_info.pNext = nullptr;
            pipeline_info.flags = 0;
            pipeline_info.stageCount = (uint32_t) shader_stages.Size();
            pipeline_info.pStages = &shader_stages[0];
            pipeline_info.pVertexInputState = &vi;
            pipeline_info.pInputAssemblyState = &ia;
            pipeline_info.pTessellationState = nullptr;
            pipeline_info.pViewportState = &vp;
            pipeline_info.pRasterizationState = &rs;
            pipeline_info.pMultisampleState = &ms;
            if (depth_attachment)
            {
                pipeline_info.pDepthStencilState = &ds;
            }
            else
            {
                pipeline_info.pDepthStencilState = nullptr;
            }
            if (color_attachment)
            {
                pipeline_info.pColorBlendState = &cb;
            }
            else
            {
                pipeline_info.pColorBlendState = nullptr;
            }
            pipeline_info.pDynamicState = &dynamic_info;
            pipeline_info.layout = pipeline_layout;
            pipeline_info.renderPass = render_pass;
            pipeline_info.subpass = 0;
            pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
            pipeline_info.basePipelineIndex = 0;

            VkResult err = vkCreateGraphicsPipelines(m_device, pipeline_cache, 1, &pipeline_info, nullptr, pipeline);
            assert(!err);
        }

        void CreateComputePipeline(
            VkShaderModule cs_module,
            VkPipelineLayout pipeline_layout,
            VkPipelineCache pipeline_cache,
            VkPipeline* pipeline)
        {
            VkPipelineShaderStageCreateInfo stage_info;
            Memory::Zero(&stage_info, sizeof(stage_info));
            stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stage_info.pNext = nullptr;
            stage_info.flags = 0;
            stage_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            stage_info.module = cs_module;
            stage_info.pName = "main";
            stage_info.pSpecializationInfo = nullptr;

            VkComputePipelineCreateInfo pipeline_info;
            Memory::Zero(&pipeline_info, sizeof(pipeline_info));
            pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
            pipeline_info.pNext = nullptr;
            pipeline_info.flags = 0;
            pipeline_info.stage = stage_info;
            pipeline_info.layout = pipeline_layout;
            pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
            pipeline_info.basePipelineIndex = 0;

            VkResult err = vkCreateComputePipelines(m_device, pipeline_cache, 1, &pipeline_info, nullptr, pipeline);
            assert(!err);
        }

        void CreateDescriptorSetPool(const Vector<UniformSet>& uniform_sets, VkDescriptorPool* descriptor_pool)
        {
            int buffer_count = 0;
            int texture_count = 0;
            int storage_image_count = 0;
            int storage_buffer_count = 0;

            for (int i = 0; i < uniform_sets.Size(); ++i)
            {
                for (int j = 0; j < uniform_sets[i].buffers.Size(); ++j)
                {
                    ++buffer_count;
                }

                for (int j = 0; j < uniform_sets[i].textures.Size(); ++j)
                {
                    if (uniform_sets[i].textures[j].stage == VK_SHADER_STAGE_COMPUTE_BIT)
                    {
                        ++storage_image_count;
                    }
                    else
                    {
                        ++texture_count;
                    }
                }

                for (int j = 0; j < uniform_sets[i].storage_buffers.Size(); ++j)
                {
                    ++storage_buffer_count;
                }
            }

            Vector<VkDescriptorPoolSize> pool_sizes;
			if (buffer_count > 0)
			{
				VkDescriptorPoolSize pool_size;
				pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				pool_size.descriptorCount = (uint32_t) buffer_count * DESCRIPTOR_POOL_SIZE_MAX;
				pool_sizes.Add(pool_size);
			}
			if (texture_count > 0)
			{
				VkDescriptorPoolSize pool_size;
				pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				pool_size.descriptorCount = (uint32_t) texture_count * DESCRIPTOR_POOL_SIZE_MAX;
				pool_sizes.Add(pool_size);
			}
            if (storage_image_count > 0)
            {
                VkDescriptorPoolSize pool_size;
                pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                pool_size.descriptorCount = (uint32_t) storage_image_count * DESCRIPTOR_POOL_SIZE_MAX;
                pool_sizes.Add(pool_size);
            }
            if (storage_buffer_count > 0)
            {
                VkDescriptorPoolSize pool_size;
                pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                pool_size.descriptorCount = (uint32_t) storage_buffer_count * DESCRIPTOR_POOL_SIZE_MAX;
                pool_sizes.Add(pool_size);
            }
            
            VkDescriptorPoolCreateInfo pool_info;
            Memory::Zero(&pool_info, sizeof(pool_info));
            pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            pool_info.pNext = nullptr;
            pool_info.flags = 0;
            pool_info.maxSets = DESCRIPTOR_POOL_SIZE_MAX;
            pool_info.poolSizeCount = (uint32_t) pool_sizes.Size();
            pool_info.pPoolSizes = &pool_sizes[0];

            VkResult err = vkCreateDescriptorPool(m_device, &pool_info, nullptr, descriptor_pool);
            assert(!err);
        }

        void CreateDescriptorSets(
            const Vector<UniformSet>& uniform_sets,
            VkDescriptorPool descriptor_pool,
            const Vector<VkDescriptorSetLayout>& descriptor_layouts,
            Vector<VkDescriptorSet>& descriptor_sets)
        {
            VkDescriptorSetAllocateInfo desc_info;
            desc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            desc_info.pNext = nullptr;
            desc_info.descriptorPool = descriptor_pool;
            desc_info.descriptorSetCount = descriptor_layouts.Size();
            desc_info.pSetLayouts = &descriptor_layouts[0];

            descriptor_sets.Resize(descriptor_layouts.Size());
            VkResult err = vkAllocateDescriptorSets(m_device, &desc_info, &descriptor_sets[0]);
            assert(!err);
        }

        void CreateUniformBuffer(VkDescriptorSet descriptor_set, UniformBuffer& buffer)
        {
            assert(!buffer.buffer);
            buffer.buffer = this->CreateBuffer(nullptr, buffer.size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

            VkDescriptorBufferInfo buffer_info;
            buffer_info.buffer = buffer.buffer->GetBuffer();
            buffer_info.offset = 0;
            buffer_info.range = buffer.size;

            VkWriteDescriptorSet desc_write;
            Memory::Zero(&desc_write, sizeof(desc_write));
            desc_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            desc_write.pNext = nullptr;
            desc_write.dstSet = descriptor_set;
            desc_write.dstBinding = buffer.binding;
            desc_write.dstArrayElement = 0;
            desc_write.descriptorCount = 1;
            desc_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            desc_write.pImageInfo = nullptr;
            desc_write.pBufferInfo = &buffer_info;
            desc_write.pTexelBufferView = nullptr;

            vkUpdateDescriptorSets(m_device, 1, &desc_write, 0, nullptr);
        }

        void UpdateUniformTexture(VkDescriptorSet descriptor_set, int binding, bool is_storage, const Ref<Texture>& texture)
        {
            VkDescriptorImageInfo image_info;
            image_info.sampler = texture->GetSampler();
            image_info.imageView = texture->GetImageView();
            image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

            VkWriteDescriptorSet desc_write;
            Memory::Zero(&desc_write, sizeof(desc_write));
            desc_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            desc_write.pNext = nullptr;
            desc_write.dstSet = descriptor_set;
            desc_write.dstBinding = binding;
            desc_write.dstArrayElement = 0;
            desc_write.descriptorCount = 1;
            desc_write.descriptorType = is_storage ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            desc_write.pImageInfo = &image_info;
            desc_write.pBufferInfo = nullptr;
            desc_write.pTexelBufferView = nullptr;

            vkUpdateDescriptorSets(m_device, 1, &desc_write, 0, nullptr);
        }

        void UpdateStorageBuffer(VkDescriptorSet descriptor_set, int binding, const Ref<BufferObject>& buffer)
        {
            VkDescriptorBufferInfo buffer_info;
            buffer_info.buffer = buffer->GetBuffer();
            buffer_info.offset = 0;
            buffer_info.range = buffer->GetSize();

            VkWriteDescriptorSet desc_write;
            Memory::Zero(&desc_write, sizeof(desc_write));
            desc_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            desc_write.pNext = nullptr;
            desc_write.dstSet = descriptor_set;
            desc_write.dstBinding = binding;
            desc_write.dstArrayElement = 0;
            desc_write.descriptorCount = 1;
            desc_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            desc_write.pImageInfo = nullptr;
            desc_write.pBufferInfo = &buffer_info;
            desc_write.pTexelBufferView = nullptr;

            vkUpdateDescriptorSets(m_device, 1, &desc_write, 0, nullptr);
        }

        void BuildInstanceCmd(
            VkCommandBuffer cmd,
            VkRenderPass render_pass,
            VkPipelineLayout pipeline_layout,
            VkPipeline pipeline,
            const Vector<VkDescriptorSet>& descriptor_sets,
            int image_width,
            int image_height,
            const Rect& view_rect,
            const Ref<BufferObject>& vertex_buffer,
            const Ref<BufferObject>& index_buffer,
            const Ref<BufferObject>& draw_buffer,
            const Ref<BufferObject>& instance_buffer)
        {
            VkCommandBufferInheritanceInfo inheritance_info;
            Memory::Zero(&inheritance_info, sizeof(inheritance_info));
            inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            inheritance_info.pNext = nullptr;
            inheritance_info.renderPass = render_pass;
            inheritance_info.subpass = 0;
            inheritance_info.framebuffer = VK_NULL_HANDLE;
            inheritance_info.occlusionQueryEnable = VK_FALSE;
            inheritance_info.queryFlags = 0;
            inheritance_info.pipelineStatistics = 0;

            VkCommandBufferBeginInfo cmd_begin;
            Memory::Zero(&cmd_begin, sizeof(cmd_begin));
            cmd_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            cmd_begin.pNext = nullptr;
            cmd_begin.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
            cmd_begin.pInheritanceInfo = &inheritance_info;

            VkResult err = vkBeginCommandBuffer(cmd, &cmd_begin);
            assert(!err);

            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
            if (descriptor_sets.Size() > 0)
            {
                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, descriptor_sets.Size(), &descriptor_sets[0], 0, nullptr);
            }

            VkViewport viewport;
            Memory::Zero(&viewport, sizeof(viewport));
            viewport.x = image_width * view_rect.x;
            viewport.y = image_height * view_rect.y;
            viewport.width = (float) image_width * view_rect.width;
            viewport.height = (float) image_height * view_rect.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(cmd, 0, 1, &viewport);

            VkRect2D scissor;
            Memory::Zero(&scissor, sizeof(scissor));
            scissor.offset.x = 0;
            scissor.offset.y = 0;
            scissor.extent.width = image_width;
            scissor.extent.height = image_height;
            vkCmdSetScissor(cmd, 0, 1, &scissor);

            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(cmd, VERTEX_INPUT_BINDING_VERTEX, 1, &vertex_buffer->GetBuffer(), &offset);
            if (instance_buffer)
            {
                vkCmdBindVertexBuffers(cmd, VERTEX_INPUT_BINDING_INSTANCE, 1, &instance_buffer->GetBuffer(), &offset);
            }
            vkCmdBindIndexBuffer(cmd, index_buffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT16);
            vkCmdDrawIndexedIndirect(cmd, draw_buffer->GetBuffer(), 0, 1, 0);

            err = vkEndCommandBuffer(cmd);
            assert(!err);
        }

        void BuildComputeInstanceCmd(
            VkCommandBuffer cmd,
            VkPipelineLayout pipeline_layout,
            VkPipeline pipeline,
            const Vector<VkDescriptorSet>& descriptor_sets,
            const Ref<BufferObject>& dispatch_buffer)
        {
            VkCommandBufferInheritanceInfo inheritance_info;
            Memory::Zero(&inheritance_info, sizeof(inheritance_info));
            inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            inheritance_info.pNext = nullptr;
            inheritance_info.renderPass = VK_NULL_HANDLE;
            inheritance_info.subpass = 0;
            inheritance_info.framebuffer = VK_NULL_HANDLE;
            inheritance_info.occlusionQueryEnable = VK_FALSE;
            inheritance_info.queryFlags = 0;
            inheritance_info.pipelineStatistics = 0;

            VkCommandBufferBeginInfo cmd_begin;
            Memory::Zero(&cmd_begin, sizeof(cmd_begin));
            cmd_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            cmd_begin.pNext = nullptr;
            cmd_begin.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            cmd_begin.pInheritanceInfo = &inheritance_info;

            VkResult err = vkBeginCommandBuffer(cmd, &cmd_begin);
            assert(!err);

            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
            if (descriptor_sets.Size() > 0)
            {
                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, descriptor_sets.Size(), &descriptor_sets[0], 0, nullptr);
            }

            vkCmdDispatchIndirect(cmd, dispatch_buffer->GetBuffer(), 0);

            err = vkEndCommandBuffer(cmd);
            assert(!err);
        }

		void BuildEmptyInstanceCmd(VkCommandBuffer cmd, VkRenderPass render_pass)
		{
			VkCommandBufferInheritanceInfo inheritance_info;
			Memory::Zero(&inheritance_info, sizeof(inheritance_info));
			inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
			inheritance_info.pNext = nullptr;
			inheritance_info.renderPass = render_pass;
			inheritance_info.subpass = 0;
			inheritance_info.framebuffer = VK_NULL_HANDLE;
			inheritance_info.occlusionQueryEnable = VK_FALSE;
			inheritance_info.queryFlags = 0;
			inheritance_info.pipelineStatistics = 0;

			VkCommandBufferBeginInfo cmd_begin;
			Memory::Zero(&cmd_begin, sizeof(cmd_begin));
			cmd_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmd_begin.pNext = nullptr;
			cmd_begin.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
			cmd_begin.pInheritanceInfo = &inheritance_info;

			VkResult err = vkBeginCommandBuffer(cmd, &cmd_begin);
			assert(!err);

			err = vkEndCommandBuffer(cmd);
			assert(!err);
		}

        void BuildEmptyComputeInstanceCmd(VkCommandBuffer cmd)
        {
            VkCommandBufferInheritanceInfo inheritance_info;
            Memory::Zero(&inheritance_info, sizeof(inheritance_info));
            inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            inheritance_info.pNext = nullptr;
            inheritance_info.renderPass = VK_NULL_HANDLE;
            inheritance_info.subpass = 0;
            inheritance_info.framebuffer = VK_NULL_HANDLE;
            inheritance_info.occlusionQueryEnable = VK_FALSE;
            inheritance_info.queryFlags = 0;
            inheritance_info.pipelineStatistics = 0;

            VkCommandBufferBeginInfo cmd_begin;
            Memory::Zero(&cmd_begin, sizeof(cmd_begin));
            cmd_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            cmd_begin.pNext = nullptr;
            cmd_begin.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            cmd_begin.pInheritanceInfo = &inheritance_info;

            VkResult err = vkBeginCommandBuffer(cmd, &cmd_begin);
            assert(!err);

            err = vkEndCommandBuffer(cmd);
            assert(!err);
        }

        void BuildPrimaryCmdBegin(VkCommandBuffer cmd)
        {
            VkCommandBufferBeginInfo cmd_begin;
            Memory::Zero(&cmd_begin, sizeof(cmd_begin));
            cmd_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            cmd_begin.pNext = nullptr;
            cmd_begin.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            cmd_begin.pInheritanceInfo = nullptr;

            VkResult err = vkBeginCommandBuffer(cmd, &cmd_begin);
            assert(!err);
        }

        void BuildPrimaryCmdEnd(VkCommandBuffer cmd)
        {
            VkResult err = vkEndCommandBuffer(cmd);
            assert(!err);
        }

        void BuildPrimaryCmd(
            VkCommandBuffer cmd,
            int swapchain_image_index,
            const Vector<VkCommandBuffer>& instance_cmds,
            VkRenderPass render_pass,
            VkFramebuffer framebuffer,
            int image_width,
            int image_height,
            CameraClearFlags clear_flag,
            const Color& clear_color,
			const Ref<Texture>& color_texture,
			const Ref<Texture>& depth_texture,
            const Vector<Ref<Texture>>& extra_color_textures)
        {
			bool color_attachment = true;
			bool depth_attachment = true;
            int sample_count = 1;
			if (color_texture || depth_texture)
			{
				color_attachment = (bool) color_texture;
				depth_attachment = (bool) depth_texture;
                if (color_attachment)
                {
                    sample_count = color_texture->GetSampleCount();
                }
                if (depth_attachment)
                {
                    sample_count = depth_texture->GetSampleCount();
                }
			}

            Vector<VkClearValue> clear_values;
            if (color_attachment)
            {
                for (int i = 0; i < 1 + extra_color_textures.Size(); ++i)
                {
                    VkClearValue clear;
                    clear.color.float32[0] = clear_color.r;
                    clear.color.float32[1] = clear_color.g;
                    clear.color.float32[2] = clear_color.b;
                    clear.color.float32[3] = clear_color.a;

                    clear_values.Add(clear);
                }
            }
            if (depth_attachment)
            {
                VkClearValue clear;
                clear.depthStencil.depth = 1.0f;
                clear.depthStencil.stencil = 0;

                clear_values.Add(clear);
            }

            VkRenderPassBeginInfo rp_begin;
            Memory::Zero(&rp_begin, sizeof(rp_begin));
            rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            rp_begin.pNext = nullptr;
            rp_begin.renderPass = render_pass;
            rp_begin.framebuffer = framebuffer;
            rp_begin.renderArea.offset.x = 0;
            rp_begin.renderArea.offset.y = 0;
            rp_begin.renderArea.extent.width = image_width;
            rp_begin.renderArea.extent.height = image_height;
            rp_begin.clearValueCount = clear_values.Size();
            rp_begin.pClearValues = &clear_values[0];

            if (color_texture || depth_texture)
            {
                if (color_texture)
                {
                    this->SetImageLayout(
                        cmd,
                        sample_count > 1 ? color_texture->GetImageMultiSample() : color_texture->GetImage(),
                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
                        VK_IMAGE_LAYOUT_GENERAL,
                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                        VK_ACCESS_SHADER_READ_BIT);
                }
                for (int i = 0; i < extra_color_textures.Size(); ++i)
                {
                    this->SetImageLayout(
                        cmd,
                        sample_count > 1 ? extra_color_textures[i]->GetImageMultiSample() : extra_color_textures[i]->GetImage(),
                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
                        VK_IMAGE_LAYOUT_GENERAL,
                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                        VK_ACCESS_SHADER_READ_BIT);
                }
                if (depth_texture)
                {
                    this->SetImageLayout(
                        cmd,
                        sample_count > 1 ? depth_texture->GetImageMultiSample() : depth_texture->GetImage(),
                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                        { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 },
                        VK_IMAGE_LAYOUT_GENERAL,
                        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                        VK_ACCESS_SHADER_READ_BIT);
                }
            }
			else
            {
                if (clear_flag == CameraClearFlags::Nothing || clear_flag == CameraClearFlags::Depth)
                {
                    this->SetImageLayout(
                        cmd,
                        m_swapchain_image_resources[swapchain_image_index].image,
                        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
                        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                        VK_ACCESS_MEMORY_READ_BIT);
                }
            }

            vkCmdBeginRenderPass(cmd, &rp_begin, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
            if (instance_cmds.Size() > 0)
            {
                vkCmdExecuteCommands(cmd, (uint32_t) instance_cmds.Size(), &instance_cmds[0]);
            }
            vkCmdEndRenderPass(cmd);

            if (sample_count > 1)
            {
                // resolve multi sample image
                if (color_texture)
                {
                    this->SetImageLayout(
                        cmd,
                        color_texture->GetImageMultiSample(),
                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                        { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
                        VK_IMAGE_LAYOUT_GENERAL,
                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        (VkAccessFlagBits) 0);

                    this->SetImageLayout(
                        cmd,
                        color_texture->GetImage(),
                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                        { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
                        VK_IMAGE_LAYOUT_GENERAL,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        (VkAccessFlagBits) 0);

                    VkImageResolve resolve;
                    Memory::Zero(&resolve, sizeof(resolve));
                    resolve.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
                    resolve.srcOffset = { 0, 0, 0 };
                    resolve.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
                    resolve.dstOffset = { 0, 0, 0 };
                    resolve.extent = { (uint32_t) color_texture->GetWidth(), (uint32_t) color_texture->GetHeight(), 1 };

                    vkCmdResolveImage(
                        cmd,
                        color_texture->GetImageMultiSample(),
                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        color_texture->GetImage(),
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        1,
                        &resolve);

                    this->SetImageLayout(
                        cmd,
                        color_texture->GetImageMultiSample(),
                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                        { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        VK_IMAGE_LAYOUT_GENERAL,
                        VK_ACCESS_TRANSFER_READ_BIT);

                    this->SetImageLayout(
                        cmd,
                        color_texture->GetImage(),
                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                        { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        VK_IMAGE_LAYOUT_GENERAL,
                        VK_ACCESS_TRANSFER_WRITE_BIT);
                }
            }
        }

        void BuildComputePrimaryCmd(
            VkCommandBuffer cmd,
            const Vector<VkCommandBuffer>& instance_cmds)
        {
            vkCmdExecuteCommands(cmd, (uint32_t) instance_cmds.Size(), &instance_cmds[0]);
        }

        void BuildPrimaryCmds()
        {
            m_cameras.Sort([](const Ref<Camera>& a, const Ref<Camera>& b) {
                return a->GetDepth() < b->GetDepth();
            });

            if (m_compute_cmd != VK_NULL_HANDLE)
            {
                VkCommandBuffer cmd = m_compute_cmd;

                this->BuildPrimaryCmdBegin(cmd);

                m_has_compute_instance_cmds = false;
                for (auto j : m_cameras)
                {
                    Vector<VkCommandBuffer> cmds = j->GetComputeInstanceCmds();
                    if (cmds.Size() > 0)
                    {
                        m_has_compute_instance_cmds = true;
                        this->BuildComputePrimaryCmd(cmd, cmds);
                    }
                }

                this->BuildPrimaryCmdEnd(cmd);
            }

            for (int i = 0; i < m_swapchain_image_resources.Size(); ++i)
            {
                VkCommandBuffer cmd = m_swapchain_image_resources[i].cmd;

                this->BuildPrimaryCmdBegin(cmd);

                for (auto j : m_cameras)
                {
                    this->BuildPrimaryCmd(
                        cmd,
                        i,
                        j->GetInstanceCmds(),
                        j->GetRenderPass(),
                        j->GetFramebuffer(i),
                        j->GetTargetWidth(),
                        j->GetTargetHeight(),
                        j->GetClearFlags(),
                        j->GetClearColor(),
						j->GetRenderTargetColor(),
						j->GetRenderTargetDepth(),
                        j->GetExtraRenderTargets());
                }

                this->BuildPrimaryCmdEnd(cmd);
            }
        }

        void Update()
        {
            for (auto i : m_cameras)
            {
                i->Update();
            }

            if (m_primary_cmd_dirty)
            {
                m_primary_cmd_dirty = false;

                this->BuildPrimaryCmds();
            }
        }

        void OnDraw()
        {
            if (m_pause_draw)
            {
                Thread::Sleep(1);
                return;
            }

            bool has_present_camera = false;
            for (auto i : m_cameras)
            {
                if (!i->HasRenderTarget())
                {
                    has_present_camera = true;
                    break;
                }
            }
            if (!has_present_camera)
            {
                return;
            }

            // wait for previous frame draw complete
            VkResult err = vkWaitForFences(m_device, 1, &m_draw_complete_fence, VK_TRUE, UINT64_MAX);
            assert(!err);
            err = vkResetFences(m_device, 1, &m_draw_complete_fence);
            assert(!err);

            this->Update();

            err = fpAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_image_acquired_semaphore, VK_NULL_HANDLE, (uint32_t*) &m_image_index);
            assert(!err);

            m_image_cmd_mutex.lock();

            if (m_compute_queue != VK_NULL_HANDLE && m_has_compute_instance_cmds)
            {
                VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
                VkSubmitInfo submit_info;
                Memory::Zero(&submit_info, sizeof(submit_info));
                submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submit_info.pNext = nullptr;
                submit_info.waitSemaphoreCount = 1;
                submit_info.pWaitSemaphores = &m_image_acquired_semaphore;
                submit_info.pWaitDstStageMask = &pipe_stage_flags;
                submit_info.commandBufferCount = 1;
                submit_info.pCommandBuffers = &m_compute_cmd;
                submit_info.signalSemaphoreCount = 1;
                submit_info.pSignalSemaphores = &m_compute_semaphore;

                err = vkQueueSubmit(m_compute_queue, 1, &submit_info, VK_NULL_HANDLE);
                assert(!err);

                pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                submit_info.pWaitSemaphores = &m_compute_semaphore;
                submit_info.pCommandBuffers = &m_swapchain_image_resources[m_image_index].cmd;
                submit_info.pSignalSemaphores = &m_draw_complete_semaphore;

                err = vkQueueSubmit(m_graphics_queue, 1, &submit_info, m_draw_complete_fence);
                assert(!err);
            }
            else
            {
                VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                VkSubmitInfo submit_info;
                Memory::Zero(&submit_info, sizeof(submit_info));
                submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submit_info.pNext = nullptr;
                submit_info.waitSemaphoreCount = 1;
                submit_info.pWaitSemaphores = &m_image_acquired_semaphore;
                submit_info.pWaitDstStageMask = &pipe_stage_flags;
                submit_info.commandBufferCount = 1;
                submit_info.pCommandBuffers = &m_swapchain_image_resources[m_image_index].cmd;
                submit_info.signalSemaphoreCount = 1;
                submit_info.pSignalSemaphores = &m_draw_complete_semaphore;

                err = vkQueueSubmit(m_graphics_queue, 1, &submit_info, m_draw_complete_fence);
                assert(!err);
            }

            VkPresentInfoKHR present_info;
            Memory::Zero(&present_info, sizeof(present_info));
            present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            present_info.pNext = nullptr;
            present_info.waitSemaphoreCount = 1;
            present_info.pWaitSemaphores = &m_draw_complete_semaphore;
            present_info.swapchainCount = 1;
            present_info.pSwapchains = &m_swapchain;
            present_info.pImageIndices = (uint32_t*) &m_image_index;
            present_info.pResults = nullptr;

            err = fpQueuePresentKHR(m_graphics_queue, &present_info);
            assert(!err);

            m_image_cmd_mutex.unlock();
        }
#endif

#if VR_GLES
        bool m_is_glesv3 = false;
        
        ~DisplayPrivate()
        {
            m_blit_mesh.reset();
            m_blit_shader.reset();
            m_cameras.Clear();
            
            this->DoneContext();

            m_public = nullptr;
            m_display = nullptr;
        }
        
        void EnableGLESv3()
        {
            m_is_glesv3 = true;
        }
        
        bool IsGLESv3() const
        {
            return m_is_glesv3;
        }

#if VR_WINDOWS
        HDC m_hdc = nullptr;
        HGLRC m_context = nullptr;
        HGLRC m_shared_context = nullptr;

        void InitContext()
        {
            m_hdc = GetDC((HWND) m_window);

            PIXELFORMATDESCRIPTOR pfd = {
                sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd
                1,                     // version number
                PFD_DRAW_TO_WINDOW |   // support window
                PFD_SUPPORT_OPENGL |   // support OpenGL
                PFD_DOUBLEBUFFER,      // double buffered
                PFD_TYPE_RGBA,         // RGBA type
                24,                    // color size
                0, 0, 0, 0, 0, 0,      // color bits ignored
                0,                     // no alpha buffer
                0,                     // shift bit ignored
                0,                     // no accumulation buffer
                0, 0, 0, 0,            // accum bits ignored
                24,                    // depth size
                0,                     // no stencil buffer
                0,                     // no auxiliary buffer
                PFD_MAIN_PLANE,        // main layer
                0,                     // reserved
                0, 0, 0                // layer masks ignored
            };
            int format_index = ChoosePixelFormat(m_hdc, &pfd);
            SetPixelFormat(m_hdc, format_index, &pfd);

            m_context = wglCreateContext(m_hdc);

            m_shared_context = wglCreateContext(m_hdc);
            wglShareLists(m_context, m_shared_context);
            
            wglMakeCurrent(m_hdc, m_context);
            
            wglewInit();
            wglSwapIntervalEXT(0);

            glewInit();

            GLint major;
            GLint minor;
            glGetIntegerv(GL_MAJOR_VERSION, &major);
            glGetIntegerv(GL_MINOR_VERSION, &minor);
            if (major >= 3)
            {
                this->EnableGLESv3();
            }
        }

        void DoneContext()
        {
            wglMakeCurrent(nullptr, nullptr);
            wglDeleteContext(m_shared_context);
            wglDeleteContext(m_context);
            ReleaseDC((HWND) m_window, m_hdc);
        }

        void SwapBuffers() const
        {
            ::SwapBuffers(m_hdc);
        }

        void BindSharedContext() const
        {
            wglMakeCurrent(m_hdc, m_shared_context);
        }

        void UnbindSharedContext() const
        {
            wglMakeCurrent(nullptr, nullptr);
        }
#elif VR_MAC
        NSOpenGLContext* m_context = nil;
        NSOpenGLContext* m_shared_context = nil;
        
        void InitContext()
        {
            m_context = [NSOpenGLContext currentContext];
            m_shared_context = [[NSOpenGLContext alloc] initWithFormat:[m_context pixelFormat] shareContext:m_context];
        }
        
        void DoneContext()
        {
            m_context = nil;
            m_shared_context = nil;
        }
        
        void SwapBuffers() const
        {
            [m_context flushBuffer];
        }
        
        void BindSharedContext() const
        {
            [m_shared_context makeCurrentContext];
        }
        
        void UnbindSharedContext() const
        {
            [NSOpenGLContext clearCurrentContext];
        }
#elif VR_IOS
        EAGLContext* m_context = nil;
        EAGLContext* m_shared_context = nil;
        Action m_bind_default_framebuffer = nullptr;
        
        void InitContext()
        {
            m_context = [EAGLContext currentContext];
            m_shared_context = [[EAGLContext alloc] initWithAPI:[m_context API] sharegroup:[m_context sharegroup]];
        }
        
        void DoneContext()
        {
            m_context = nil;
            m_shared_context = nil;
        }
        
        void SwapBuffers() const
        {
            
        }
        
        void BindSharedContext() const
        {
            [EAGLContext setCurrentContext:m_shared_context];
        }
        
        void UnbindSharedContext() const
        {
            [EAGLContext setCurrentContext:nil];
        }
        
        void SetBindDefaultFramebufferImplemment(Action action)
        {
            m_bind_default_framebuffer = action;
        }
        
        void BindDefaultFramebuffer() const
        {
            if (m_bind_default_framebuffer)
            {
                m_bind_default_framebuffer();
            }
        }
#elif VR_ANDROID
        EGLDisplay m_egl_display = EGL_NO_DISPLAY;
        EGLConfig m_egl_config = nullptr;
        EGLSurface m_egl_surface = EGL_NO_SURFACE;
        EGLContext m_context = EGL_NO_CONTEXT;
        EGLContext m_shared_context = EGL_NO_CONTEXT;

        void InitContext()
        {
            m_egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
            if (m_egl_display == EGL_NO_DISPLAY)
            {
                return;
            }

            EGLBoolean ret = eglInitialize(m_egl_display, nullptr, nullptr);
            if (ret == EGL_FALSE)
            {
                return;
            }

            const EGLint config_attribs[] = {
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_RED_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_BLUE_SIZE, 8,
                EGL_DEPTH_SIZE, 24,
                EGL_SAMPLE_BUFFERS, EGL_FALSE,
                EGL_SAMPLES, 0,
                EGL_NONE
            };

            int config_count;
            ret = eglChooseConfig(m_egl_display, config_attribs, &m_egl_config, 1, &config_count);
            if (ret == EGL_FALSE || config_count == 0)
            {
                Log("no egl config count choose");
                return;
            }

            m_egl_surface = eglCreateWindowSurface(m_egl_display, m_egl_config, (EGLNativeWindowType) m_window, nullptr);

            EGLint context_attribs[] = {
                EGL_CONTEXT_CLIENT_VERSION, 3,
                EGL_NONE
            };

            m_context = eglCreateContext(m_egl_display, m_egl_config, nullptr, context_attribs);
            if (m_context == EGL_NO_CONTEXT)
            {
                // down grade to OpenGL ES 2.0
                context_attribs[1] = 2;
                m_context = eglCreateContext(m_egl_display, m_egl_config, nullptr, context_attribs);
            }
            else
            {
                this->EnableGLESv3();
            }

            m_shared_context = eglCreateContext(m_egl_display, m_egl_config, m_context, context_attribs);

            eglMakeCurrent(m_egl_display, m_egl_surface, m_egl_surface, m_context);
        }

        void DoneContext()
        {
            eglMakeCurrent(m_egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            eglDestroyContext(m_egl_display, m_context);
            eglDestroyContext(m_egl_display, m_shared_context);
            if (m_egl_surface != EGL_NO_SURFACE)
            {
                eglDestroySurface(m_egl_display, m_egl_surface);
            }
            eglTerminate(m_egl_display);
        }

        void SwapBuffers() const
        {
            eglSwapBuffers(m_egl_display, m_egl_surface);
        }

        void BindSharedContext() const
        {
            eglMakeCurrent(m_egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, m_shared_context);
        }

        void UnbindSharedContext() const
        {
            eglMakeCurrent(m_egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        }
#elif VR_UWP
        void InitContext()
        {

        }

        void DoneContext()
        {

        }

        void SwapBuffers() const
        {

        }

        void BindSharedContext() const
        {
            Viry3D::BindSharedContext();
        }

        void UnbindSharedContext() const
        {
            Viry3D::UnbindSharedContext();
        }
#else
        void InitContext()
        {

        }

        void DoneContext()
        {

        }

        void SwapBuffers() const
        {

        }

        void BindSharedContext() const
        {

        }

        void UnbindSharedContext() const
        {

        }
#endif

#if VR_ANDROID
        void OnPause()
        {
            eglMakeCurrent(m_egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, m_context);
            eglDestroySurface(m_egl_display, m_egl_surface);
            m_egl_surface = EGL_NO_SURFACE;
        }

        void OnResume()
        {
            m_egl_surface = eglCreateWindowSurface(m_egl_display, m_egl_config, (EGLNativeWindowType) m_window, nullptr);
            eglMakeCurrent(m_egl_display, m_egl_surface, m_egl_surface, m_context);
        }
#else
        void OnPause()
        {

        }

        void OnResume()
        {

        }
#endif
        
#if VR_WASM
        Display::Platform m_platform = Display::Platform::Other;
        
        void SetPlatform(Display::Platform platform)
        {
            m_platform = platform;
        }
        
        Display::Platform GetPlatform() const
        {
            return m_platform;
        }
#endif

        int GetMaxSamples()
        {
            if (this->IsGLESv3())
            {
                GLint count;
                glGetIntegerv(GL_MAX_SAMPLES, &count);
                return count;
            }
            else
            {
                return 1;
            }
        }

        Ref<BufferObject> CreateBuffer(const void* data, int size, GLenum target, GLenum usage)
        {
            Ref<BufferObject> buffer = RefMake<BufferObject>(size);
            buffer->m_target = target;
            buffer->m_usage = usage;

            glGenBuffers(1, &buffer->m_buffer);
            glBindBuffer(target, buffer->m_buffer);
            glBufferData(target, size, data, usage);
            glBindBuffer(target, 0);

            return buffer;
        }

        void OnResize(int width, int height)
        {
            m_width = width;
            m_height = height;

            for (auto i : m_cameras)
            {
                i->OnResize(m_width, m_height);
            }
        }

        void OnDraw()
        {
            m_cameras.Sort([](const Ref<Camera>& a, const Ref<Camera>& b) {
                return a->GetDepth() < b->GetDepth();
            });

            for (auto i : m_cameras)
            {
                i->Update();
            }

            for (auto i : m_cameras)
            {
                i->OnDraw();
            }

            this->SwapBuffers();
        }
#endif

        void OnFrameEnd()
        {
            List<Ref<Camera>> cameras = m_cameras;
            for (auto i : cameras)
            {
                i->OnFrameEnd();
            }
        }

        void CreateBlitShader()
        {
#if VR_VULKAN
            String vs = R"(
Input(0) vec3 a_pos;
Input(2) vec2 a_uv;

Output(0) vec2 v_uv;

void main()
{
	gl_Position = vec4(a_pos, 1.0);
	v_uv = a_uv;

	vulkan_convert();
}
)";
            String fs = R"(
precision highp float;
      
UniformTexture(0, 0) uniform sampler2D u_texture;

Input(0) vec2 v_uv;

Output(0) vec4 o_frag;

void main()
{
    o_frag = texture(u_texture, v_uv);
}
)";
#elif VR_GLES
            String vs = R"(
uniform int u_flip_y;

attribute vec3 a_pos;
attribute vec2 a_uv;

varying vec2 v_uv;

void main()
{
	gl_Position = vec4(a_pos, 1.0);
	v_uv = a_uv;

    if (u_flip_y == 1)
    {
        v_uv.y = 1.0 - v_uv.y;
    }
}
)";
            String fs = R"(
precision highp float;
      
uniform sampler2D u_texture;

varying vec2 v_uv;

void main()
{
    gl_FragColor = texture2D(u_texture, v_uv);
}
)";
#endif
            RenderState render_state;
            render_state.cull = RenderState::Cull::Off;
            render_state.zTest = RenderState::ZTest::Off;
            render_state.zWrite = RenderState::ZWrite::Off;

            m_blit_shader = RefMake<Shader>(
                "",
                Vector<String>(),
                vs,
                "",
                Vector<String>(),
                fs,
                render_state);
        }

        void CreateBlitMesh()
        {
            Vector<Vertex> vertices(4);
            Memory::Zero(&vertices[0], vertices.SizeInBytes());
            vertices[0].vertex = Vector3(-1.0f, 1.0f, 0);
            vertices[1].vertex = Vector3(-1.0f, -1.0f, 0);
            vertices[2].vertex = Vector3(1.0f, -1.0f, 0);
            vertices[3].vertex = Vector3(1.0f, 1.0f, 0);
            vertices[0].uv = Vector2(0, 0);
            vertices[1].uv = Vector2(0, 1);
            vertices[2].uv = Vector2(1, 1);
            vertices[3].uv = Vector2(1, 0);

            Vector<unsigned short> indices({ 0, 1, 2, 0, 2, 3 });

            m_blit_mesh = RefMake<Mesh>(vertices, indices);
        }
    };

    Display* DisplayPrivate::m_display;

    Display* Display::Instance()
    {
        return DisplayPrivate::m_display;
    }

    Display::Display(const String& name, void* window, int width, int height):
        m_private(new DisplayPrivate(this, window, width, height))
    {
#if VR_VULKAN
#if VR_WINDOWS
        InitShaderCompiler();
#elif VR_ANDROID
        InitVulkan();
#endif
        
        m_private->CheckInstanceLayers();
        m_private->CheckInstanceExtensions();
        m_private->CreateInstance(name);
        m_private->CreateDebugReportCallback();
        m_private->InitPhysicalDevice();
        m_private->CreateSurface();
        m_private->CreateDevice();
        m_private->GetQueues();
        m_private->CreateSignals();
        m_private->CreateImageCmd();
        m_private->CreateComputeCmd();
        m_private->CreateSizeDependentResources();
#elif VR_GLES
        String version = (const char*) glGetString(GL_VERSION);
        Log("GL Version: %s", version.CString());

#if VR_WINDOWS || VR_MAC
        glClearDepth(1.0f);
#else
        glClearDepthf(1.0f);
#endif

        glClearStencil(0);
        glFrontFace(GL_CCW);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#endif
    }

    Display::~Display()
    {
        delete m_private;
        
#if VR_VULKAN
#if VR_WINDOWS
        DeinitShaderCompiler();
#endif
#endif
    }

    void Display::OnResize(int width, int height)
    {
        m_private->OnResize(width, height);
    }

    void Display::OnPause()
    {
        m_private->OnPause();
    }

    void Display::OnResume()
    {
        m_private->OnResume();
    }

    void Display::OnDraw()
    {
        m_private->OnDraw();
        m_private->OnFrameEnd();
    }

    int Display::GetWidth() const
    {
        return m_private->m_width;
    }

    int Display::GetHeight() const
    {
        return m_private->m_height;
    }

    Camera* Display::CreateCamera()
    {
        Ref<Camera> camera = RefMake<Camera>();
        m_private->m_cameras.AddLast(camera);

#if VR_VULKAN
        this->MarkPrimaryCmdDirty();
#endif

        return camera.get();
    }

    Camera* Display::CreateBlitCamera(int depth, const Ref<Texture>& texture, const Ref<Material>& material, const String& texture_name, CameraClearFlags clear_flags, const Rect& rect)
    {
        Ref<Material> blit_material = material;

        if (!blit_material)
        {
            if (!m_private->m_blit_shader)
            {
                m_private->CreateBlitShader();
            }
            blit_material = RefMake<Material>(m_private->m_blit_shader);
        }

        if (!m_private->m_blit_mesh)
        {
            m_private->CreateBlitMesh();
        }

        Ref<MeshRenderer> renderer = RefMake<MeshRenderer>();
        renderer->SetMaterial(blit_material);
        renderer->SetMesh(m_private->m_blit_mesh);

        Camera* camera = this->CreateCamera();
        camera->SetViewportRect(rect);
        camera->SetClearFlags(clear_flags);
        camera->SetDepth(depth);
        camera->AddRenderer(renderer);

        if (texture)
        {
            String blit_texture_name = texture_name;
            if (blit_texture_name.Empty())
            {
                blit_texture_name = "u_texture";
            }
            blit_material->SetTexture(blit_texture_name, texture);
            
#if VR_GLES
            blit_material->SetInt("u_flip_y", texture->IsRenderTexture() ? 1 : 0);
#endif
        }

        return camera;
    }

    void Display::DestroyCamera(Camera* camera)
    {
#if VR_VULKAN
        this->WaitDevice();
#endif

        for (const auto& i : m_private->m_cameras)
        {
            if (i.get() == camera)
            {
                m_private->m_cameras.Remove(i);
                break;
            }
        }

#if VR_VULKAN
        this->MarkPrimaryCmdDirty();
#endif
    }

    int Display::GetMaxSamples()
    {
        return m_private->GetMaxSamples();
    }

#if VR_VULKAN
    VkDevice Display::GetDevice() const
    {
        return m_private->m_device;
    }

    void Display::WaitDevice() const
    {
        vkDeviceWaitIdle(m_private->m_device);
    }

    void Display::MarkPrimaryCmdDirty()
    {
        m_private->m_primary_cmd_dirty = true;
    }

    void Display::CreateRenderPass(
        const Ref<Texture>& color_texture,
        const Ref<Texture>& depth_texture,
        const Vector<Ref<Texture>>& extra_color_textures,
        CameraClearFlags clear_flag,
        bool stereo_rendering,
        VkRenderPass* render_pass,
        Vector<VkFramebuffer>& framebuffers)
    {
        bool present = !(color_texture || depth_texture);

        if (present)
        {
            m_private->CreateRenderPass(
                m_private->m_swapchain_image_resources[0].format,
                m_private->m_depth_texture->GetFormat(),
                Vector<VkFormat>(),
                clear_flag,
                1,
                true,
                stereo_rendering,
                render_pass);

            framebuffers.Resize(m_private->m_swapchain_image_resources.Size());
            for (int i = 0; i < framebuffers.Size(); ++i)
            {
                const SwapchainImageResources& resource = m_private->m_swapchain_image_resources[i];

                m_private->CreateFramebuffer(
                    resource.image_view,
                    m_private->m_depth_texture->GetImageView(),
                    Vector<VkImageView>(),
                    resource.width,
                    resource.height,
                    *render_pass,
                    &framebuffers[i]);
            }
        }
        else
        {
            VkFormat color_format = VK_FORMAT_UNDEFINED;
            VkFormat depth_format = VK_FORMAT_UNDEFINED;
            VkImageView color_image_view = VK_NULL_HANDLE;
            VkImageView depth_image_view = VK_NULL_HANDLE;
            int image_width = 0;
            int image_height = 0;
            int sample_count = 1;

            if (color_texture)
            {
                sample_count = color_texture->GetSampleCount();
                color_format = color_texture->GetFormat();
                image_width = color_texture->GetWidth();
                image_height = color_texture->GetHeight();
                if (sample_count > 1)
                {
                    color_image_view = color_texture->GetImageViewMultiSample();
                }
                else
                {
                    color_image_view = color_texture->GetImageView();
                }
            }

            if (depth_texture)
            {
                sample_count = depth_texture->GetSampleCount();
                depth_format = depth_texture->GetFormat();
                image_width = depth_texture->GetWidth();
                image_height = depth_texture->GetHeight();
                if (sample_count > 1)
                {
                    depth_image_view = depth_texture->GetImageViewMultiSample();
                }
                else
                {
                    depth_image_view = depth_texture->GetImageView();
                }
            }

            Vector<VkFormat> extra_color_formats(extra_color_textures.Size());
            Vector<VkImageView> extra_color_image_views(extra_color_textures.Size());

            for (int i = 0; i < extra_color_textures.Size(); ++i)
            {
                sample_count = extra_color_textures[i]->GetSampleCount();
                extra_color_formats[i] = extra_color_textures[i]->GetFormat();
                if (sample_count > 1)
                {
                    extra_color_image_views[i] = extra_color_textures[i]->GetImageViewMultiSample();
                }
                else
                {
                    extra_color_image_views[i] = extra_color_textures[i]->GetImageView();
                }
            }

            m_private->CreateRenderPass(
                color_format,
                depth_format,
                extra_color_formats,
                clear_flag,
                sample_count,
                false,
                stereo_rendering,
                render_pass);

            framebuffers.Resize(1);
            m_private->CreateFramebuffer(
                color_image_view,
                depth_image_view,
                extra_color_image_views,
                image_width,
                image_height,
                *render_pass,
                &framebuffers[0]);
        }
    }

    void Display::CreateCommandPool(VkCommandPool* cmd_pool)
    {
        m_private->CreateCommandPool(m_private->m_graphics_queue_family_index, cmd_pool);
    }
    
    void Display::CreateComputeCommandPool(VkCommandPool* cmd_pool)
    {
        m_private->CreateCommandPool(m_private->m_compute_queue_family_index, cmd_pool);
    }

    void Display::CreateCommandBuffer(VkCommandPool cmd_pool, VkCommandBufferLevel level, VkCommandBuffer* cmd)
    {
        m_private->CreateCommandBuffer(cmd_pool, level, cmd);
    }

    void Display::CreateShaderModule(
        const String& vs_predefine,
        const Vector<String>& vs_includes,
        const String& vs_source,
        const String& fs_predefine,
        const Vector<String>& fs_includes,
        const String& fs_source,
        VkShaderModule* vs_module,
        VkShaderModule* fs_module,
        Vector<VertexAttribute>& attributes,
        Vector<UniformSet>& uniform_sets)
    {
        m_private->CreateShaderModule(
            vs_predefine,
            vs_includes,
            vs_source,
            fs_predefine,
            fs_includes,
            fs_source,
            vs_module,
            fs_module,
            attributes,
            uniform_sets);
    }

    void Display::CreateComputeShaderModule(
        const String& cs_source,
        VkShaderModule* cs_module,
        Vector<UniformSet>& uniform_sets)
    {
        m_private->CreateComputeShaderModule(
            cs_source,
            cs_module,
            uniform_sets);
    }

    void Display::CreatePipelineCache(VkPipelineCache* pipeline_cache)
    {
        m_private->CreatePipelineCache(pipeline_cache);
    }

    void Display::CreatePipelineLayout(
        const Vector<UniformSet>& uniform_sets,
        Vector<VkDescriptorSetLayout>& descriptor_layouts,
        VkPipelineLayout* pipeline_layout)
    {
        m_private->CreatePipelineLayout(uniform_sets, descriptor_layouts, pipeline_layout);
    }

    void Display::CreatePipeline(
        VkRenderPass render_pass,
        const Vector<VertexAttribute>& attributes,
        VkShaderModule vs_module,
        VkShaderModule fs_module,
        const RenderState& render_state,
        VkPipelineLayout pipeline_layout,
        VkPipelineCache pipeline_cache,
        VkPipeline* pipeline,
        bool color_attachment,
        bool depth_attachment,
        int extra_color_attachment_count,
        int sample_count,
        bool instancing,
        int instance_stride)
    {
        m_private->CreatePipeline(
            render_pass,
            attributes,
            vs_module,
            fs_module,
            render_state,
            pipeline_layout,
            pipeline_cache,
            pipeline,
            color_attachment,
            depth_attachment,
            extra_color_attachment_count,
            sample_count,
            instancing,
            instance_stride);
    }

    void Display::CreateComputePipeline(
        VkShaderModule cs_module,
        VkPipelineLayout pipeline_layout,
        VkPipelineCache pipeline_cache,
        VkPipeline* pipeline)
    {
        m_private->CreateComputePipeline(
            cs_module,
            pipeline_layout,
            pipeline_cache,
            pipeline);
    }

    void Display::CreateDescriptorSetPool(const Vector<UniformSet>& uniform_sets, VkDescriptorPool* descriptor_pool)
    {
        m_private->CreateDescriptorSetPool(uniform_sets, descriptor_pool);
    }

    void Display::CreateDescriptorSets(
        const Vector<UniformSet>& uniform_sets,
        VkDescriptorPool descriptor_pool,
        const Vector<VkDescriptorSetLayout>& descriptor_layouts,
        Vector<VkDescriptorSet>& descriptor_sets)
    {
        m_private->CreateDescriptorSets(
            uniform_sets,
            descriptor_pool,
            descriptor_layouts,
            descriptor_sets);
    }

    void Display::CreateUniformBuffer(VkDescriptorSet descriptor_set, UniformBuffer& buffer)
    {
        m_private->CreateUniformBuffer(descriptor_set, buffer);
    }

    void Display::UpdateUniformTexture(VkDescriptorSet descriptor_set, int binding, bool is_storage, const Ref<Texture>& texture)
    {
        m_private->UpdateUniformTexture(descriptor_set, binding, is_storage, texture);
    }

    void Display::UpdateStorageBuffer(VkDescriptorSet descriptor_set, int binding, const Ref<BufferObject>& buffer)
    {
        m_private->UpdateStorageBuffer(descriptor_set, binding, buffer);
    }

    Ref<BufferObject> Display::CreateBuffer(const void* data, int size, VkBufferUsageFlags usage)
    {
        return m_private->CreateBuffer(data, size, usage);
    }

    void Display::UpdateBuffer(const Ref<BufferObject>& buffer, int buffer_offset, const void* data, int size)
    {
        m_private->UpdateBuffer(buffer, buffer_offset, data, size);
    }

    void Display::ReadBuffer(const Ref<BufferObject>& buffer, ByteBuffer& data)
    {
        m_private->ReadBuffer(buffer, data);
    }

    void Display::BuildInstanceCmd(
        VkCommandBuffer cmd,
        VkRenderPass render_pass,
        VkPipelineLayout pipeline_layout,
        VkPipeline pipeline,
        const Vector<VkDescriptorSet>& descriptor_sets,
        int image_width,
        int image_height,
        const Rect& view_rect,
        const Ref<BufferObject>& vertex_buffer,
        const Ref<BufferObject>& index_buffer,
        const Ref<BufferObject>& draw_buffer,
        const Ref<BufferObject>& instance_buffer)
    {
        m_private->BuildInstanceCmd(
            cmd,
            render_pass,
            pipeline_layout,
            pipeline,
            descriptor_sets,
            image_width,
            image_height,
            view_rect,
            vertex_buffer,
            index_buffer,
            draw_buffer,
            instance_buffer);
    }

    void Display::BuildComputeInstanceCmd(
        VkCommandBuffer cmd,
        VkPipelineLayout pipeline_layout,
        VkPipeline pipeline,
        const Vector<VkDescriptorSet>& descriptor_sets,
        const Ref<BufferObject>& dispatch_buffer)
    {
        m_private->BuildComputeInstanceCmd(
            cmd,
            pipeline_layout,
            pipeline,
            descriptor_sets,
            dispatch_buffer);
    }

	void Display::BuildEmptyInstanceCmd(VkCommandBuffer cmd, VkRenderPass render_pass)
	{
		m_private->BuildEmptyInstanceCmd(cmd, render_pass);
	}

    void Display::BuildEmptyComputeInstanceCmd(VkCommandBuffer cmd)
    {
        m_private->BuildEmptyComputeInstanceCmd(cmd);
    }

    VkFormat Display::ChooseFormatSupported(const Vector<VkFormat>& formats, VkFormatFeatureFlags features)
    {
        return m_private->ChooseFormatSupported(formats, features);
    }

    Ref<Texture> Display::CreateTexture(
        VkImageType type,
        VkImageViewType view_type,
        int width,
        int height,
        VkFormat format,
        VkImageUsageFlags usage,
        VkImageAspectFlags aspect_flag,
        const VkComponentMapping& component,
        int mipmap_level_count,
        bool cubemap,
        int array_size,
        int sample_count)
    {
        return m_private->CreateTexture(
            type,
            view_type,
            width,
            height,
            format,
            usage,
            aspect_flag,
            component,
            mipmap_level_count,
            cubemap,
            array_size,
            sample_count);
    }

    void Display::CreateSampler(
        const Ref<Texture>& texture,
        VkFilter filter_mode,
        VkSamplerAddressMode wrap_mode)
    {
        m_private->CreateSampler(texture, filter_mode, wrap_mode);
    }

    void Display::BeginImageCmd()
    {
        m_private->BeginImageCmd();
    }

    void Display::EndImageCmd()
    {
        m_private->EndImageCmd();
    }

    void Display::SetImageLayout(
        VkImage image,
		VkPipelineStageFlags src_stage,
		VkPipelineStageFlags dst_stage,
        const VkImageSubresourceRange& subresource_range,
        VkImageLayout old_image_layout,
        VkImageLayout new_image_layout,
        VkAccessFlagBits src_access_mask)
    {
        m_private->SetImageLayout(
			m_private->m_image_cmd,
            image,
			src_stage,
			dst_stage,
            subresource_range,
            old_image_layout,
            new_image_layout,
            src_access_mask);
    }

    VkCommandBuffer Display::GetImageCmd() const
    {
        return m_private->m_image_cmd;
    }

    bool Display::IsSupportMultiview() const
    {
        return m_private->m_has_multiview_extension && m_private->m_gpu_features.multiViewport == VK_TRUE;
    }
#elif VR_GLES
    void Display::EnableGLESv3()
    {
        m_private->EnableGLESv3();
    }
    
    bool Display::IsGLESv3() const
    {
        return m_private->IsGLESv3();
    }
    
    Ref<BufferObject> Display::CreateBuffer(const void* data, int size, GLenum target, GLenum usage)
    {
        return m_private->CreateBuffer(data, size, target, usage);
    }

    void Display::UpdateBuffer(const Ref<BufferObject>& buffer, int buffer_offset, const void* data, int size)
    {
        glBindBuffer(buffer->GetTarget(), buffer->GetBuffer());
        glBufferSubData(buffer->GetTarget(), buffer_offset, size, data);
        glBindBuffer(buffer->GetTarget(), 0);
    }

    void Display::BindSharedContext() const
    {
        m_private->BindSharedContext();
    }

    void Display::UnbindSharedContext() const
    {
        m_private->UnbindSharedContext();
    }

    void Display::Flush() const
    {
        glFlush();
    }
    
#if VR_IOS
    void Display::SetBindDefaultFramebufferImplemment(Action action)
    {
        m_private->SetBindDefaultFramebufferImplemment(action);
    }
    
    void Display::BindDefaultFramebuffer() const
    {
        m_private->BindDefaultFramebuffer();
    }
#elif VR_WASM
    void Display::SetPlatform(Platform platform)
    {
        m_private->SetPlatform(platform);
    }
    
    Display::Platform Display::GetPlatform() const
    {
        return m_private->GetPlatform();
    }
#endif
#endif
}
