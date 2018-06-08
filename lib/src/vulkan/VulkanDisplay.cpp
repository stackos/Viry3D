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

#include "VulkanDisplay.h"
#include "vulkan_include.h"
#include "vulkan_shader_compiler.h"
#include "Application.h"
#include "Debug.h"
#include "container/Vector.h"
#include "string/String.h"
#include "memory/Memory.h"
#include "graphics/RenderTexture.h"
#include "graphics/CameraClearFlags.h"
#include "graphics/Color.h"
#include "graphics/VertexAttribute.h"
#include "math/Rect.h"
#include "io/File.h"
#include <assert.h>

extern "C"
{
#include "crypto/md5/md5.h"
}

static PFN_vkGetDeviceProcAddr g_gdpa = nullptr;

#define GET_INSTANCE_PROC_ADDR(inst, entrypoint)                                                              \
    {                                                                                                         \
        fp##entrypoint = (PFN_vk##entrypoint) vkGetInstanceProcAddr(inst, "vk" #entrypoint);                  \
    }

#define GET_DEVICE_PROC_ADDR(dev, entrypoint)                                                                    \
    {                                                                                                            \
        if (!g_gdpa) g_gdpa = (PFN_vkGetDeviceProcAddr) vkGetInstanceProcAddr(m_instance, "vkGetDeviceProcAddr");\
        fp##entrypoint = (PFN_vk##entrypoint) g_gdpa(dev, "vk" #entrypoint);                                     \
    }

#define FRAME_LAG 2
#define VSYNC 0

namespace Viry3D
{
    struct VkTexture
    {
        int width;
        int height;
        VkFormat format;
        VkImage image;
        VkImageView image_view;
        VkDeviceMemory memory;
        VkMemoryAllocateInfo memory_info;
        bool is_ref_image;

        VkTexture():
            width(0),
            height(0),
            format(VK_FORMAT_UNDEFINED),
            image(nullptr),
            image_view(nullptr),
            memory(nullptr),
            is_ref_image(true)
        {
            Memory::Zero(&memory_info, sizeof(memory_info));
        }
        
        void Destroy(VkDevice device)
        {
            if (is_ref_image == false)
            {
                vkDestroyImage(device, image, nullptr);
                vkDestroyImageView(device, image_view, nullptr);
                vkFreeMemory(device, memory, nullptr);
            }
        }
    };

    struct VkBufferObject
    {
        VkBuffer buffer;
        VkDeviceMemory memory;
        VkMemoryAllocateInfo memory_info;
        int size;

        VkBufferObject():
            buffer(nullptr),
            memory(nullptr),
            size(0)
        {
            Memory::Zero(&memory_info, sizeof(memory_info));
        }

        void Destroy(VkDevice device)
        {
            vkDestroyBuffer(device, buffer, nullptr);
            vkFreeMemory(device, memory, nullptr);
        }
    };

    struct SwapchainImageResources
    {
        Ref<VkTexture> texture;
        VkCommandBuffer cmd;

        SwapchainImageResources():
            cmd(nullptr)
        {
        }
    };

    class VulkanDisplayPrivate
    {
    public:
        VulkanDisplay * m_public;
        void* m_window;
        int m_width;
        int m_height;
        Vector<char*> m_enabled_layers;
        Vector<char*> m_instance_extension_names;
        Vector<char*> m_device_extension_names;
        VkInstance m_instance = nullptr;
        VkDebugReportCallbackEXT m_debug_callback = nullptr;
        VkPhysicalDevice m_gpu = nullptr;
        VkSurfaceKHR m_surface = nullptr;
        VkDevice m_device = nullptr;
        VkQueue m_graphics_queue = nullptr;
        VkPhysicalDeviceProperties m_gpu_properties;
        Vector<VkQueueFamilyProperties> m_queue_properties;
        VkPhysicalDeviceFeatures m_gpu_features;
        VkPhysicalDeviceMemoryProperties m_memory_properties;
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
        VkSurfaceFormatKHR m_surface_format;
        VkFence m_fences[FRAME_LAG];
        VkSemaphore m_image_acquired_semaphores[FRAME_LAG];
        VkSemaphore m_draw_complete_semaphores[FRAME_LAG];
        int m_frame_index = 0;
        VkSwapchainKHR m_swapchain = nullptr;
        Vector<SwapchainImageResources> m_swapchain_image_resources;
        uint32_t m_image_index = 0;
        VkCommandPool m_graphics_cmd_pool = nullptr;
        VkCommandBuffer m_image_cmd = nullptr;
        Ref<VkTexture> m_depth_texture;
        VkRenderPass m_render_pass = nullptr;
        Vector<VkFramebuffer> m_framebuffers;
        VkPipelineCache m_pipeline_cache = nullptr;
        VkDescriptorSetLayout m_desc_layout = nullptr;
        VkPipelineLayout m_pipeline_layout = nullptr;
        VkPipeline m_pipeline = nullptr;
        Ref<VkBufferObject> m_vertex_buffer;
        Ref<VkBufferObject> m_index_buffer;

        VulkanDisplayPrivate(VulkanDisplay* device, void* window, int width, int height):
            m_public(device),
            m_window(window),
            m_width(width),
            m_height(height)
        {
            Memory::Zero(m_fences, sizeof(m_fences));
            Memory::Zero(m_image_acquired_semaphores, sizeof(m_image_acquired_semaphores));
            Memory::Zero(m_draw_complete_semaphores, sizeof(m_draw_complete_semaphores));
        }

        ~VulkanDisplayPrivate()
        {
            vkDeviceWaitIdle(m_device);

            this->DestroySizeDependentResources();

            for (int i = 0; i < FRAME_LAG; ++i)
            {
                vkWaitForFences(m_device, 1, &m_fences[i], VK_TRUE, UINT64_MAX);
                vkDestroyFence(m_device, m_fences[i], nullptr);
                vkDestroySemaphore(m_device, m_image_acquired_semaphores[i], nullptr);
                vkDestroySemaphore(m_device, m_draw_complete_semaphores[i], nullptr);
            }

            vkDestroyDevice(m_device, nullptr);
            vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
            m_gpu = nullptr;
            fpDestroyDebugReportCallbackEXT(m_instance, m_debug_callback, nullptr);
            vkDestroyInstance(m_instance, nullptr);
            VulkanDisplayPrivate::StringVectorClear(m_enabled_layers);
            VulkanDisplayPrivate::StringVectorClear(m_instance_extension_names);
            VulkanDisplayPrivate::StringVectorClear(m_device_extension_names);
            m_public = nullptr;
        }

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
            for (int i = 0; i < sizeof(hash_bytes); i++)
            {
                md5_str += String::Format("%02x", hash_bytes[i]);
            }

            String cache_path = Application::SavePath() + "/" + md5_str + ".cache";
            if (File::Exist(cache_path))
            {
                auto buffer = File::ReadAllBytes(cache_path);
                spirv.Resize(buffer.Size() / 4);
                Memory::Copy(&spirv[0], buffer.Bytes(), buffer.Size());
            }
            else
            {
                String error;
                bool success = glsl_to_spv(shader_type, glsl.CString(), spirv, error);
                assert(success);

                ByteBuffer buffer(spirv.SizeInBytes());
                Memory::Copy(buffer.Bytes(), &spirv[0], buffer.Size());
                File::WriteAllBytes(cache_path, buffer);
            }
        }

        static String ProcessShaderSource(const String& glsl, const Vector<String>& includes)
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
            for (const auto& i : includes)
            {
                auto include_path = Application::DataPath() + "/shader/Include/" + i;
                auto bytes = File::ReadAllBytes(include_path);
                auto include_str = String(bytes);
                source += include_str + "\n";
            }
            source += glsl;

            return source;
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
                "VK_LAYER_LUNARG_standard_validation"
            };
            
            const char** instance_validation_layers = instance_validation_layers_alt1;
            int instance_validation_layer_count = sizeof(instance_validation_layers_alt1) / sizeof(instance_validation_layers_alt1[0]);

            bool validation_found = check_layer(
                instance_validation_layers,
                instance_validation_layer_count,
                &instance_layers[0],
                instance_layers.Size());
            if (!validation_found)
            {
                const char* instance_validation_layers_alt2[] = {
                    "VK_LAYER_GOOGLE_threading",
                    "VK_LAYER_LUNARG_parameter_validation",
                    "VK_LAYER_LUNARG_object_tracker",
                    "VK_LAYER_LUNARG_core_validation",
                    "VK_LAYER_LUNARG_image",
                    "VK_LAYER_LUNARG_swapchain",
                    "VK_LAYER_GOOGLE_unique_objects"
                };

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
                const char* instance_validation_layers_alt3[] = {
                    "VK_LAYER_NV_optimus"
                };

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
                    VulkanDisplayPrivate::StringVectorAdd(m_enabled_layers, instance_validation_layers[i]);
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
                    VulkanDisplayPrivate::StringVectorAdd(m_instance_extension_names, VK_KHR_SURFACE_EXTENSION_NAME);
                }
#if defined(VK_USE_PLATFORM_WIN32_KHR)
                if (strcmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName) == 0)
                {
                    platform_surface_ext_found = true;
                    VulkanDisplayPrivate::StringVectorAdd(m_instance_extension_names, VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
                }
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
                if (strcmp(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName) == 0)
                {
                    platform_surface_ext_found = true;
                    VulkanDisplayPrivate::StringVectorAdd(m_instance_extension_names, VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
                }
#elif defined(VK_USE_PLATFORM_IOS_MVK)
                if (strcmp(VK_MVK_IOS_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName) == 0)
                {
                    platform_surface_ext_found = true;
                    VulkanDisplayPrivate::StringVectorAdd(m_instance_extension_names, VK_MVK_IOS_SURFACE_EXTENSION_NAME);
                }
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
                if (strcmp(VK_MVK_MACOS_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName) == 0)
                {
                    platform_surface_ext_found = true;
                    VulkanDisplayPrivate::StringVectorAdd(m_instance_extension_names, VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
                }
#endif
                if (strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, instance_extensions[i].extensionName) == 0)
                {
                    platform_surface_ext_found = true;
                    VulkanDisplayPrivate::StringVectorAdd(m_instance_extension_names, VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
                }
            }

            assert(surface_ext_found);
            assert(platform_surface_ext_found);
        }

        void CreateInstance()
        {
            String name = Application::Current()->GetName();

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
            inst_info.enabledLayerCount = m_enabled_layers.Size();
            inst_info.ppEnabledLayerNames = &m_enabled_layers[0];
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

        static VKAPI_ATTR VkBool32 VKAPI_CALL
            DebugFunc(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType,
                uint64_t srcObject, size_t location, int32_t msgCode,
                const char *pLayerPrefix, const char *pMsg, void *pUserData)
        {
            Log("[%s] %d : %s", pLayerPrefix, msgCode, pMsg);

            String message;

            if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
            {
                message = String::Format("ERROR: [%s] Code %d : %s", pLayerPrefix, msgCode, pMsg);
            }
            else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
            {
                message = String::Format("WARNING: [%s] Code %d : %s", pLayerPrefix, msgCode, pMsg);
            }
            else
            {
                return false;
            }

#if VR_WINDOWS
            MessageBox(NULL, message.CString(), "Alert", MB_OK);
#endif

            return false;
        }

        void CreateDebugReportCallback()
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
                    VulkanDisplayPrivate::StringVectorAdd(m_device_extension_names, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
                }
            }
            assert(swapchain_ext_found);

            vkGetPhysicalDeviceProperties(m_gpu, &m_gpu_properties);

            uint32_t queue_family_count;
            vkGetPhysicalDeviceQueueFamilyProperties(m_gpu, &queue_family_count, nullptr);
            assert(queue_family_count > 0);

            m_queue_properties.Resize(queue_family_count);
            vkGetPhysicalDeviceQueueFamilyProperties(m_gpu, &queue_family_count, &m_queue_properties[0]);
            vkGetPhysicalDeviceFeatures(m_gpu, &m_gpu_features);
            vkGetPhysicalDeviceMemoryProperties(m_gpu, &m_memory_properties);
        }

        void CreateDevice()
        {
            VkResult err;

#if defined(VK_USE_PLATFORM_WIN32_KHR)
            VkWin32SurfaceCreateInfoKHR create_info;
            Memory::Zero(&create_info, sizeof(create_info));
            create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
            create_info.pNext = nullptr;
            create_info.flags = 0;
            create_info.hinstance = GetModuleHandle(nullptr);
            create_info.hwnd = (HWND) m_window;
            
            err = vkCreateWin32SurfaceKHR(m_instance, &create_info, nullptr, &m_surface);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
            VkAndroidSurfaceCreateInfoKHR create_info;
            Memory::Zero(&create_info, sizeof(create_info));
            create_info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
            create_info.pNext = nullptr;
            create_info.flags = 0;
            create_info.window = (ANativeWindow*) m_window;

            err = vkCreateAndroidSurfaceKHR(m_instance, &create_info, nullptr, &m_surface);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
            VkIOSSurfaceCreateInfoMVK create_info;
            Memory::Zero(&create_info, sizeof(create_info));
            create_info.sType = VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK;
            create_info.pNext = nullptr;
            create_info.flags = 0;
            create_info.pView = m_window;

            err = vkCreateIOSSurfaceMVK(m_instance, &create_info, nullptr, &m_surface);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
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

            float queue_priorities[1] = { 0.0 };
            VkDeviceQueueCreateInfo queue_infos[2];
            Memory::Zero(queue_infos, sizeof(queue_infos));
            queue_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_infos[0].pNext = nullptr;
            queue_infos[0].flags = 0;
            queue_infos[0].queueFamilyIndex = m_graphics_queue_family_index;
            queue_infos[0].queueCount = 1;
            queue_infos[0].pQueuePriorities = queue_priorities;

            VkDeviceCreateInfo device_info;
            Memory::Zero(&device_info, sizeof(device_info));
            device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            device_info.pNext = nullptr;
            device_info.flags = 0;
            device_info.queueCreateInfoCount = 1;
            device_info.pQueueCreateInfos = queue_infos;
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

            vkGetDeviceQueue(m_device, m_graphics_queue_family_index, 0, &m_graphics_queue);

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

        void CreateSemaphores()
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

            VkResult err;
            for (int i = 0; i < FRAME_LAG; ++i)
            {
                err = vkCreateFence(m_device, &fence_info, nullptr, &m_fences[i]);
                assert(!err);

                err = vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_image_acquired_semaphores[i]);
                assert(!err);

                err = vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_draw_complete_semaphores[i]);
                assert(!err);
            }
        }

        void CreateSizeDependentResources()
        {
            this->CreateSwapChain();
            this->CreateCommandBuffer();
            m_depth_texture = this->CreateTexture(
                VK_IMAGE_TYPE_2D,
                VK_IMAGE_VIEW_TYPE_2D,
                m_width,
                m_height,
                VK_FORMAT_D32_SFLOAT,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_IMAGE_ASPECT_DEPTH_BIT,
                {
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY
                });
            
            this->CreateRenderPasses();
            this->CreatePipelineCache();
            this->CreatePipeline();
            this->CreateVertexBuffer();
        }

        void DestroySizeDependentResources()
        {
            m_vertex_buffer->Destroy(m_device);
            m_vertex_buffer.reset();
            m_index_buffer->Destroy(m_device);
            m_index_buffer.reset();

            vkDestroyPipeline(m_device, m_pipeline, nullptr);
            vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
            vkDestroyDescriptorSetLayout(m_device, m_desc_layout, nullptr);
            vkDestroyPipelineCache(m_device, m_pipeline_cache, nullptr);

            for (int i = 0; i < m_swapchain_image_resources.Size(); ++i)
            {
                vkDestroyFramebuffer(m_device, m_framebuffers[i], nullptr);
            }
            m_framebuffers.Clear();
            vkDestroyRenderPass(m_device, m_render_pass, nullptr);

            m_depth_texture->Destroy(m_device);
            m_depth_texture.reset();

            for (int i = 0; i < m_swapchain_image_resources.Size(); ++i)
            {
                vkFreeCommandBuffers(m_device, m_graphics_cmd_pool, 1, &m_swapchain_image_resources[i].cmd);
            }
            vkFreeCommandBuffers(m_device, m_graphics_cmd_pool, 1, &m_image_cmd);
            vkDestroyCommandPool(m_device, m_graphics_cmd_pool, nullptr);

            for (int i = 0; i < m_swapchain_image_resources.Size(); ++i)
            {
                vkDestroyImageView(m_device, m_swapchain_image_resources[i].texture->image_view, nullptr);
            }
            m_swapchain_image_resources.Clear();
            fpDestroySwapchainKHR(m_device, m_swapchain, nullptr);
            m_swapchain = nullptr;
        }

        void OnResize(int width, int height)
        {
            m_width = width;
            m_height = height;

            vkDeviceWaitIdle(m_device);

            this->DestroySizeDependentResources();
            this->CreateSizeDependentResources();
        }

        void CreateSwapChain()
        {
            VkResult err;

            VkSurfaceCapabilitiesKHR surface_caps;
            err = fpGetPhysicalDeviceSurfaceCapabilitiesKHR(m_gpu, m_surface, &surface_caps);
            assert(!err);

            VkExtent2D swapchain_size;
            if (surface_caps.currentExtent.width == 0xFFFFFFFF)
            {
                swapchain_size.width = m_width;
                swapchain_size.height = m_height;

                if (swapchain_size.width < surface_caps.minImageExtent.width)
                {
                    swapchain_size.width = surface_caps.minImageExtent.width;
                }
                else if (swapchain_size.width > surface_caps.maxImageExtent.width)
                {
                    swapchain_size.width = surface_caps.maxImageExtent.width;
                }

                if (swapchain_size.height < surface_caps.minImageExtent.height)
                {
                    swapchain_size.height = surface_caps.minImageExtent.height;
                }
                else if (swapchain_size.height > surface_caps.maxImageExtent.height)
                {
                    swapchain_size.height = surface_caps.maxImageExtent.height;
                }
            }
            else
            {
                swapchain_size = surface_caps.currentExtent;
            }
            assert(swapchain_size.width > 0 && swapchain_size.height > 0);

            uint32_t present_mode_count;
            err = fpGetPhysicalDeviceSurfacePresentModesKHR(m_gpu, m_surface, &present_mode_count, nullptr);
            assert(!err);
            assert(present_mode_count > 0);

            Vector<VkPresentModeKHR> present_modes(present_mode_count);
            err = fpGetPhysicalDeviceSurfacePresentModesKHR(m_gpu, m_surface, &present_mode_count, &present_modes[0]);
            assert(!err);

#if VSYNC || VR_ANDROID
            VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
#else
            VkPresentModeKHR present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
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
            swapchain_info.clipped = true;
            swapchain_info.oldSwapchain = old_swapchain;

            err = fpCreateSwapchainKHR(m_device, &swapchain_info, nullptr, &m_swapchain);
            assert(!err);

            if (old_swapchain != nullptr)
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

                m_swapchain_image_resources[i].texture = RefMake<VkTexture>();
                m_swapchain_image_resources[i].texture->is_ref_image = true;
                m_swapchain_image_resources[i].texture->image = swapchain_images[i];

                err = vkCreateImageView(m_device, &view_info, nullptr, &m_swapchain_image_resources[i].texture->image_view);
                assert(!err);

                m_swapchain_image_resources[i].texture->format = m_surface_format.format;
                m_swapchain_image_resources[i].texture->width = swapchain_size.width;
                m_swapchain_image_resources[i].texture->height = swapchain_size.height;
            }
        }

        void CreateCommandBuffer()
        {
            VkCommandPoolCreateInfo pool_info;
            Memory::Zero(&pool_info, sizeof(pool_info));
            pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            pool_info.pNext = nullptr;
            pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            pool_info.queueFamilyIndex = m_graphics_queue_family_index;
            
            VkResult err = vkCreateCommandPool(m_device, &pool_info, nullptr, &m_graphics_cmd_pool);
            assert(!err);

            VkCommandBufferAllocateInfo cmd_info;
            Memory::Zero(&cmd_info, sizeof(cmd_info));
            cmd_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            cmd_info.pNext = nullptr;
            cmd_info.commandPool = m_graphics_cmd_pool;
            cmd_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            cmd_info.commandBufferCount = 1;

            err = vkAllocateCommandBuffers(m_device, &cmd_info, &m_image_cmd);
            assert(!err);

            for (int i = 0; i < m_swapchain_image_resources.Size(); ++i)
            {
                err = vkAllocateCommandBuffers(m_device, &cmd_info, &m_swapchain_image_resources[i].cmd);
                assert(!err);
            }
        }

        void CreateRenderPass(
            VkFormat color_format,
            VkFormat depth_format,
            CameraClearFlags clear_flag,
            bool present,
            VkRenderPass* render_pass)
        {
            VkAttachmentLoadOp color_load;
            VkAttachmentLoadOp depth_load;
            VkImageLayout color_final_layout;

            switch (clear_flag)
            {
                case CameraClearFlags::Color:
                {
                    color_load = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    depth_load = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    break;
                }
                case CameraClearFlags::Depth:
                {
                    color_load = VK_ATTACHMENT_LOAD_OP_LOAD;
                    depth_load = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    break;
                }
                case CameraClearFlags::Invalidate:
                {
                    color_load = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    depth_load = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    break;
                }
                default:
                {
                    color_load = VK_ATTACHMENT_LOAD_OP_LOAD;
                    depth_load = VK_ATTACHMENT_LOAD_OP_LOAD;
                    break;
                }
            }

            if (present)
            {
                color_final_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            }
            else
            {
                color_final_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }

            Vector<VkAttachmentDescription> attachments;

            attachments.Add(VkAttachmentDescription());
            Memory::Zero(&attachments[0], sizeof(attachments[0]));
            attachments[0].flags = 0;
            attachments[0].format = color_format;
            attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[0].loadOp = color_load;
            attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[0].finalLayout = color_final_layout;

            if (depth_format != VK_FORMAT_UNDEFINED)
            {
                attachments.Add(VkAttachmentDescription());
                Memory::Zero(&attachments[1], sizeof(attachments[1]));
                attachments[1].flags = 0;
                attachments[1].format = depth_format;
                attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
                attachments[1].loadOp = depth_load;
                attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                attachments[1].stencilLoadOp = depth_load;
                attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
                attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            }

            VkAttachmentReference color_reference = {
                0,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            };
            VkAttachmentReference depth_reference = {
                1,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            };

            VkSubpassDescription subpass;
            Memory::Zero(&subpass, sizeof(subpass));
            subpass.flags = 0;
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.inputAttachmentCount = 0;
            subpass.pInputAttachments = nullptr;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &color_reference;
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

            VkSubpassDependency dependencies[2];
            Memory::Zero(dependencies, sizeof(dependencies));

            dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[0].dstSubpass = 0;
            dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            dependencies[1].srcSubpass = 0;
            dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            VkRenderPassCreateInfo rp_info;
            Memory::Zero(&rp_info, sizeof(rp_info));
            rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            rp_info.pNext = nullptr;
            rp_info.flags = 0;
            rp_info.attachmentCount = attachments.Size();
            rp_info.pAttachments = &attachments[0];
            rp_info.subpassCount = 1;
            rp_info.pSubpasses = &subpass;
            rp_info.dependencyCount = 2;
            rp_info.pDependencies = dependencies;

            VkResult err = vkCreateRenderPass(m_device, &rp_info, nullptr, render_pass);
            assert(!err);
        }

        void CreateFramebuffer(
            VkImageView color_image_view,
            VkImageView depth_image_view,
            int image_width,
            int image_height,
            VkRenderPass render_pass,
            VkFramebuffer* framebuffer)
        {
            Vector<VkImageView> attachments_view;
            attachments_view.Add(color_image_view);
            if (depth_image_view != nullptr)
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

        Ref<VkTexture> CreateTexture(
            VkImageType type,
            VkImageViewType view_type,
            int width,
            int height,
            VkFormat format,
            VkImageUsageFlags usage,
            VkImageAspectFlags aspect_flag,
            const VkComponentMapping& component)
        {
            Ref<VkTexture> texture = RefMake<VkTexture>();
            texture->is_ref_image = false;
            texture->width = width;
            texture->height = height;
            texture->format = format;

            VkImageCreateInfo image_info;
            Memory::Zero(&image_info, sizeof(image_info));
            image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image_info.pNext = nullptr;
            image_info.flags = 0;
            image_info.imageType = type;
            image_info.format = format;
            image_info.extent = { (uint32_t) width, (uint32_t) height, 1 };
            image_info.mipLevels = 1;
            image_info.arrayLayers = 1;
            image_info.samples = VK_SAMPLE_COUNT_1_BIT;
            image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
            image_info.usage = usage;
            image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            image_info.queueFamilyIndexCount = 0;
            image_info.pQueueFamilyIndices = nullptr;
            image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            VkResult err = vkCreateImage(m_device, &image_info, nullptr, &texture->image);
            assert(!err);

            VkMemoryRequirements mem_reqs;
            vkGetImageMemoryRequirements(m_device, texture->image, &mem_reqs);

            Memory::Zero(&texture->memory_info, sizeof(texture->memory_info));
            texture->memory_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            texture->memory_info.pNext = nullptr;
            texture->memory_info.allocationSize = mem_reqs.size;
            texture->memory_info.memoryTypeIndex = 0;

            bool pass = this->CheckMemoryType(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture->memory_info.memoryTypeIndex);
            assert(pass);

            err = vkAllocateMemory(m_device, &texture->memory_info, nullptr, &texture->memory);
            assert(!err);

            err = vkBindImageMemory(m_device, texture->image, texture->memory, 0);
            assert(!err);

            VkImageViewCreateInfo view_info;
            Memory::Zero(&view_info, sizeof(view_info));
            view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            view_info.pNext = nullptr;
            view_info.flags = 0;
            view_info.image = texture->image;
            view_info.viewType = view_type;
            view_info.format = format;
            view_info.components = component;
            view_info.subresourceRange = { aspect_flag, 0, 1, 0, 1 };
            
            err = vkCreateImageView(m_device, &view_info, nullptr, &texture->image_view);
            assert(!err);

            return texture;
        }

        Ref<VkBufferObject> CreateBuffer(const void* data, int size, VkBufferUsageFlags usage)
        {
            Ref<VkBufferObject> buffer = RefMake<VkBufferObject>();
            buffer->size = size;

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

            VkResult err = vkCreateBuffer(m_device, &buffer_info, nullptr, &buffer->buffer);
            assert(!err);

            VkMemoryRequirements mem_reqs;
            vkGetBufferMemoryRequirements(m_device, buffer->buffer, &mem_reqs);

            Memory::Zero(&buffer->memory_info, sizeof(buffer->memory_info));
            buffer->memory_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            buffer->memory_info.pNext = nullptr;
            buffer->memory_info.allocationSize = mem_reqs.size;
            buffer->memory_info.memoryTypeIndex = 0;

            bool pass = this->CheckMemoryType(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer->memory_info.memoryTypeIndex);
            assert(pass);

            err = vkAllocateMemory(m_device, &buffer->memory_info, nullptr, &buffer->memory);
            assert(!err);

            void* map_data = nullptr;
            err = vkMapMemory(m_device, buffer->memory, 0, VK_WHOLE_SIZE, 0, (void**) &map_data);
            assert(!err);

            Memory::Copy(map_data, data, size);

            vkUnmapMemory(m_device, buffer->memory);

            err = vkBindBufferMemory(m_device, buffer->buffer, buffer->memory, 0);
            assert(!err);

            return buffer;
        }

        void CreateRenderPasses()
        {
            this->CreateRenderPass(
                m_swapchain_image_resources[0].texture->format,
                m_depth_texture->format,
                CameraClearFlags::Color,
                true,
                &m_render_pass);

            m_framebuffers.Resize(m_swapchain_image_resources.Size());
            for (int i = 0; i < m_swapchain_image_resources.Size(); ++i)
            {
                this->CreateFramebuffer(
                    m_swapchain_image_resources[i].texture->image_view,
                    m_depth_texture->image_view,
                    m_swapchain_image_resources[i].texture->width,
                    m_swapchain_image_resources[i].texture->height,
                    m_render_pass,
                    &m_framebuffers[i]);
            }
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

        void CreateGlslShaderModule(const String& glsl, VkShaderStageFlagBits shader_type, VkShaderModule* module)
        {
            Vector<unsigned int> spirv;
            VulkanDisplayPrivate::GlslToSpirvCached(glsl, shader_type, spirv);
            this->CreateSpirvShaderModule(spirv, module);
        }

        void CreatePipelineCache()
        {
            VkPipelineCacheCreateInfo pipeline_cache;
            Memory::Zero(&pipeline_cache, sizeof(pipeline_cache));
            pipeline_cache.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

            VkResult err;
            err = vkCreatePipelineCache(m_device, &pipeline_cache, nullptr, &m_pipeline_cache);
            assert(!err);
        }

        void CreatePipeline()
        {
            Vector<String> vs_includes;
            vs_includes.Add("Base.in");

            String vs = R"(
UniformBuffer(0, 0) uniform UniformBuffer00
{
	mat4 mvp;
} u_buf_0_0;

Input(0) vec4 a_pos;
Input(1) vec2 a_uv;

Output(0) vec2 v_uv;

void main()
{
	//gl_Position = a_pos * u_buf_0_0.mvp;
    gl_Position = a_pos;
	v_uv = a_uv;

	vulkan_convert();
}
)";
            String fs = R"(
precision mediump float;
      
UniformTexture(0, 1) uniform sampler2D u_texture;

Input(0) vec2 v_uv;

Output(0) vec4 o_frag;

void main()
{
    //o_frag = texture(u_texture, v_uv);
    o_frag = vec4(1, 1, 1, 1);
}
)";
            vs = VulkanDisplayPrivate::ProcessShaderSource(vs, vs_includes);
            fs = VulkanDisplayPrivate::ProcessShaderSource(fs, Vector<String>());

            VkShaderModule vs_module;
            VkShaderModule fs_module;
            this->CreateGlslShaderModule(vs, VK_SHADER_STAGE_VERTEX_BIT, &vs_module);
            this->CreateGlslShaderModule(fs, VK_SHADER_STAGE_FRAGMENT_BIT, &fs_module);

            Vector<VkPipelineShaderStageCreateInfo> shader_stages(2);
            Memory::Zero(&shader_stages[0], shader_stages.SizeInBytes());
            shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shader_stages[0].pNext = nullptr;
            shader_stages[0].flags = 0;
            shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
            shader_stages[0].module = vs_module;
            shader_stages[0].pName = "main";
            shader_stages[0].pSpecializationInfo = nullptr;

            shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shader_stages[1].pNext = nullptr;
            shader_stages[1].flags = 0;
            shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shader_stages[1].module = fs_module;
            shader_stages[1].pName = "main";
            shader_stages[1].pSpecializationInfo = nullptr;

            VkVertexInputBindingDescription vi_bind;
            Memory::Zero(&vi_bind, sizeof(vi_bind));
            vi_bind.binding = 0;
            vi_bind.stride = sizeof(Vertex);
            vi_bind.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            Vector<VkVertexInputAttributeDescription> vi_attrs((int) VertexAttributeType::Count);
            Memory::Zero(&vi_attrs[0], vi_attrs.SizeInBytes());
            int location = (int) VertexAttributeType::Vertex;
            vi_attrs[location].location = location;
            vi_attrs[location].binding = 0;
            vi_attrs[location].format = VK_FORMAT_R32G32B32_SFLOAT;
            vi_attrs[location].offset = VERTEX_ATTR_OFFSETS[location];
            location = (int) VertexAttributeType::Color;
            vi_attrs[location].location = location;
            vi_attrs[location].binding = 0;
            vi_attrs[location].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            vi_attrs[location].offset = VERTEX_ATTR_OFFSETS[location];
            location = (int) VertexAttributeType::Texcoord;
            vi_attrs[location].location = location;
            vi_attrs[location].binding = 0;
            vi_attrs[location].format = VK_FORMAT_R32G32_SFLOAT;
            vi_attrs[location].offset = VERTEX_ATTR_OFFSETS[location];
            location = (int) VertexAttributeType::Texcoord2;
            vi_attrs[location].location = location;
            vi_attrs[location].binding = 0;
            vi_attrs[location].format = VK_FORMAT_R32G32_SFLOAT;
            vi_attrs[location].offset = VERTEX_ATTR_OFFSETS[location];
            location = (int) VertexAttributeType::Normal;
            vi_attrs[location].location = location;
            vi_attrs[location].binding = 0;
            vi_attrs[location].format = VK_FORMAT_R32G32B32_SFLOAT;
            vi_attrs[location].offset = VERTEX_ATTR_OFFSETS[location];
            location = (int) VertexAttributeType::Tangent;
            vi_attrs[location].location = location;
            vi_attrs[location].binding = 0;
            vi_attrs[location].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            vi_attrs[location].offset = VERTEX_ATTR_OFFSETS[location];
            location = (int) VertexAttributeType::BlendWeight;
            vi_attrs[location].location = location;
            vi_attrs[location].binding = 0;
            vi_attrs[location].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            vi_attrs[location].offset = VERTEX_ATTR_OFFSETS[location];
            location = (int) VertexAttributeType::BlendIndices;
            vi_attrs[location].location = location;
            vi_attrs[location].binding = 0;
            vi_attrs[location].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            vi_attrs[location].offset = VERTEX_ATTR_OFFSETS[location];

            VkPipelineVertexInputStateCreateInfo vi;
            Memory::Zero(&vi, sizeof(vi));
            vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vi.pNext = nullptr;
            vi.flags = 0;
            vi.vertexBindingDescriptionCount = 1;
            vi.pVertexBindingDescriptions = &vi_bind;
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
            rs.cullMode = VK_CULL_MODE_BACK_BIT;
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
            ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
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
            ds.depthTestEnable = VK_TRUE;
            ds.depthWriteEnable = VK_TRUE;
            ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
            ds.depthBoundsTestEnable = VK_FALSE;
            ds.stencilTestEnable = VK_FALSE;
            ds.front = { VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_NEVER, 0, 0, 0 };
            ds.back = ds.front;
            ds.minDepthBounds = 0;
            ds.maxDepthBounds = 0;

            Vector<VkPipelineColorBlendAttachmentState> att_state(1);
            Memory::Zero(&att_state[0], att_state.SizeInBytes());
            att_state[0].blendEnable = VK_FALSE;
            att_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            att_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            att_state[0].colorBlendOp = VK_BLEND_OP_ADD;
            att_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            att_state[0].alphaBlendOp = VK_BLEND_OP_ADD;
            att_state[0].colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT |
                VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT |
                VK_COLOR_COMPONENT_A_BIT;

            VkPipelineColorBlendStateCreateInfo cb;
            Memory::Zero(&cb, sizeof(cb));
            cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            cb.pNext = nullptr;
            cb.flags = 0;
            cb.logicOpEnable = VK_FALSE;
            cb.logicOp = VK_LOGIC_OP_CLEAR;
            cb.attachmentCount = (uint32_t) att_state.Size();
            cb.pAttachments = &att_state[0];
            cb.blendConstants[0] = 0;
            cb.blendConstants[1] = 0;
            cb.blendConstants[2] = 0;
            cb.blendConstants[3] = 0;

            Vector<VkDescriptorSetLayoutBinding> layout_bindings(2);
            Memory::Zero(&layout_bindings[0], layout_bindings.SizeInBytes());
            layout_bindings[0].binding = 0;
            layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            layout_bindings[0].descriptorCount = 1;
            layout_bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            layout_bindings[0].pImmutableSamplers = nullptr;
            layout_bindings[1].binding = 1;
            layout_bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            layout_bindings[1].descriptorCount = 1;
            layout_bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            layout_bindings[1].pImmutableSamplers = nullptr;

            VkDescriptorSetLayoutCreateInfo descriptor_layout;
            Memory::Zero(&descriptor_layout, sizeof(descriptor_layout));
            descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            descriptor_layout.pNext = nullptr;
            descriptor_layout.flags = 0;
            descriptor_layout.bindingCount = layout_bindings.Size();
            descriptor_layout.pBindings = &layout_bindings[0];

            VkResult err = vkCreateDescriptorSetLayout(m_device, &descriptor_layout, nullptr, &m_desc_layout);
            assert(!err);

            VkPipelineLayoutCreateInfo pipeline_layout_info;
            Memory::Zero(&pipeline_layout_info, sizeof(pipeline_layout_info));
            pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_info.pNext = nullptr;
            pipeline_layout_info.flags = 0;
            pipeline_layout_info.setLayoutCount = 1;
            pipeline_layout_info.pSetLayouts = &m_desc_layout;
            pipeline_layout_info.pushConstantRangeCount = 0;
            pipeline_layout_info.pPushConstantRanges = nullptr;

            err = vkCreatePipelineLayout(m_device, &pipeline_layout_info, nullptr, &m_pipeline_layout);
            assert(!err);

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
            pipeline_info.pDepthStencilState = &ds;
            pipeline_info.pColorBlendState = &cb;
            pipeline_info.pDynamicState = &dynamic_info;
            pipeline_info.layout = m_pipeline_layout;
            pipeline_info.renderPass = m_render_pass;
            pipeline_info.subpass = 0;
            pipeline_info.basePipelineHandle = nullptr;
            pipeline_info.basePipelineIndex = 0;

            err = vkCreateGraphicsPipelines(m_device, m_pipeline_cache, 1, &pipeline_info, nullptr, &m_pipeline);
            assert(!err);

            vkDestroyShaderModule(m_device, vs_module, nullptr);
            vkDestroyShaderModule(m_device, fs_module, nullptr);
        }

        void CreateVertexBuffer()
        {
            Vertex vertices[3];
            Memory::Zero(vertices, sizeof(vertices));
            vertices[0].vertex = Vector3(0, 0, 0);
            vertices[1].vertex = Vector3(0, -0.5f, 0);
            vertices[2].vertex = Vector3(0.5f, -0.5f, 0);

            unsigned short indices[] = {
                0, 1, 2
            };

            m_vertex_buffer = this->CreateBuffer(&vertices[0], sizeof(vertices), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
            m_index_buffer = this->CreateBuffer(&indices[0], sizeof(indices), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        }

        void BuildCmd(
            VkCommandBuffer cmd,
            VkRenderPass render_pass,
            VkFramebuffer framebuffer,
            const Color& clear_color,
            const Rect& view_rect)
        {
            VkCommandBufferBeginInfo cmd_begin;
            Memory::Zero(&cmd_begin, sizeof(cmd_begin));
            cmd_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            cmd_begin.pNext = nullptr;
            cmd_begin.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            cmd_begin.pInheritanceInfo = nullptr;

            VkResult err = vkBeginCommandBuffer(cmd, &cmd_begin);
            assert(!err);

            VkClearValue clear_values[2];
            clear_values[0].color.float32[0] = clear_color.r;
            clear_values[0].color.float32[1] = clear_color.g;
            clear_values[0].color.float32[2] = clear_color.b;
            clear_values[0].color.float32[3] = clear_color.a;
            clear_values[1].depthStencil.depth = 1.0f;
            clear_values[1].depthStencil.stencil = 0;

            VkRenderPassBeginInfo rp_begin;
            Memory::Zero(&rp_begin, sizeof(rp_begin));
            rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            rp_begin.pNext = nullptr;
            rp_begin.renderPass = render_pass;
            rp_begin.framebuffer = framebuffer;
            rp_begin.renderArea.offset.x = 0;
            rp_begin.renderArea.offset.y = 0;
            rp_begin.renderArea.extent.width = m_width;
            rp_begin.renderArea.extent.height = m_height;
            rp_begin.clearValueCount = 2;
            rp_begin.pClearValues = clear_values;

            vkCmdBeginRenderPass(cmd, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
            //vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 0, 1, &m_descriptor_set, 0, nullptr);

            VkViewport viewport;
            Memory::Zero(&viewport, sizeof(viewport));
            viewport.x = m_width * view_rect.x;
            viewport.y = m_height * view_rect.y;
            viewport.width = (float) m_width * view_rect.width;
            viewport.height = (float) m_height * view_rect.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(cmd, 0, 1, &viewport);

            VkRect2D scissor;
            Memory::Zero(&scissor, sizeof(scissor));
            scissor.offset.x = 0;
            scissor.offset.y = 0;
            scissor.extent.width = m_width;
            scissor.extent.height = m_height;
            vkCmdSetScissor(cmd, 0, 1, &scissor);

            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(cmd, 0, 1, &m_vertex_buffer->buffer, &offset);
            vkCmdBindIndexBuffer(cmd, m_index_buffer->buffer, 0, VK_INDEX_TYPE_UINT16);
            vkCmdDrawIndexed(cmd, 3, 1, 0, 0, 0);

            vkCmdEndRenderPass(cmd);

            err = vkEndCommandBuffer(cmd);
            assert(!err);
        }

        void OnDraw()
        {
            vkWaitForFences(m_device, 1, &m_fences[m_frame_index], VK_TRUE, UINT64_MAX);
            vkResetFences(m_device, 1, &m_fences[m_frame_index]);

            VkResult err = fpAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_image_acquired_semaphores[m_frame_index], nullptr, &m_image_index);
            assert(!err);

            this->BuildCmd(
                m_swapchain_image_resources[m_image_index].cmd,
                m_render_pass,
                m_framebuffers[m_image_index],
                Color(0, 0, 0, 1),
                Rect(0, 0, 1, 1));

            VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            VkSubmitInfo submit_info;
            Memory::Zero(&submit_info, sizeof(submit_info));
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.pNext = nullptr;
            submit_info.waitSemaphoreCount = 1;
            submit_info.pWaitSemaphores = &m_image_acquired_semaphores[m_frame_index];
            submit_info.pWaitDstStageMask = &pipe_stage_flags;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &m_swapchain_image_resources[m_image_index].cmd;
            submit_info.signalSemaphoreCount = 1;
            submit_info.pSignalSemaphores = &m_draw_complete_semaphores[m_frame_index];

            err = vkQueueSubmit(m_graphics_queue, 1, &submit_info, m_fences[m_frame_index]);
            assert(!err);

            VkPresentInfoKHR present_info;
            Memory::Zero(&present_info, sizeof(present_info));
            present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            present_info.pNext = nullptr;
            present_info.waitSemaphoreCount = 1;
            present_info.pWaitSemaphores = &m_draw_complete_semaphores[m_frame_index];
            present_info.swapchainCount = 1;
            present_info.pSwapchains = &m_swapchain;
            present_info.pImageIndices = &m_image_index;
            present_info.pResults = nullptr;

            err = fpQueuePresentKHR(m_graphics_queue, &present_info);
            assert(!err);

            m_frame_index += 1;
            m_frame_index %= FRAME_LAG;
        }
    };

    VulkanDisplay::VulkanDisplay(void* window, int width, int height):
        m_private(new VulkanDisplayPrivate(this, window, width, height))
    {
        m_private->CheckInstanceLayers();
        m_private->CheckInstanceExtensions();
        m_private->CreateInstance();
        m_private->CreateDebugReportCallback();
        m_private->InitPhysicalDevice();
        m_private->CreateDevice();
        m_private->CreateSemaphores();
        m_private->CreateSizeDependentResources();
    }

    VulkanDisplay::~VulkanDisplay()
    {
        delete m_private;
    }

    void VulkanDisplay::OnResize(int width, int height)
    {
        m_private->OnResize(width, height);
    }

    void VulkanDisplay::OnDraw()
    {
        m_private->OnDraw();
    }
}
