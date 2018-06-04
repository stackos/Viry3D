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
#include "container/Vector.h"
#include "string/String.h"
#include "memory/Memory.h"
#include "graphics/RenderTexture.h"
#include "graphics/CameraClearFlags.h"
#include "math/Rect.h"
#include "Application.h"
#include "Debug.h"
#include <assert.h>

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
    struct SwapchainImageResources
    {
        VkImage image;
        VkImageView image_view;
        VkFormat format;
        int width;
        int height;
        VkCommandBuffer cmd;

        SwapchainImageResources():
            image(nullptr),
            image_view(nullptr),
            format(VK_FORMAT_UNDEFINED),
            width(0),
            height(0),
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
        VkSwapchainKHR m_swapchain = nullptr;
        Vector<SwapchainImageResources> m_swapchain_image_resources;
        VkCommandPool m_graphics_cmd_pool = nullptr;
        VkCommandBuffer m_image_cmd = nullptr;
        Ref<RenderTexture> m_depth_texture;
        VkPipelineCache m_pipeline_cache = nullptr;

        Vector<VkRenderPass> m_render_passes;
        Vector<VkFramebuffer> m_framebuffers;

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
            StringVectorClear(m_enabled_layers);
            StringVectorClear(m_instance_extension_names);
            StringVectorClear(m_device_extension_names);
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
#if defined(VK_USE_PLATFORM_WIN32_KHR)
                if (strcmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName) == 0)
                {
                    platform_surface_ext_found = true;
                    StringVectorAdd(m_instance_extension_names, VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
                }
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
                if (strcmp(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName) == 0)
                {
                    platform_surface_ext_found = true;
                    StringVectorAdd(m_instance_extension_names, VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
                }
#elif defined(VK_USE_PLATFORM_IOS_MVK)
                if (strcmp(VK_MVK_IOS_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName) == 0)
                {
                    platform_surface_ext_found = true;
                    StringVectorAdd(m_instance_extension_names, VK_MVK_IOS_SURFACE_EXTENSION_NAME);
                }
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
                if (strcmp(VK_MVK_MACOS_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName) == 0)
                {
                    platform_surface_ext_found = true;
                    StringVectorAdd(m_instance_extension_names, VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
                }
#endif
                if (strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, instance_extensions[i].extensionName) == 0)
                {
                    platform_surface_ext_found = true;
                    StringVectorAdd(m_instance_extension_names, VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
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
                    StringVectorAdd(m_device_extension_names, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
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
            this->CreatePipelineCache();

            m_depth_texture = RenderTexture::Create(
                m_width, m_height,
                RenderTextureFormat::Depth,
                DepthBuffer::Depth_24_Stencil_8,
                FilterMode::Point);

            this->CreateRenderPasses();
        }

        void DestroySizeDependentResources()
        {
            for (int i = 0; i < m_swapchain_image_resources.Size(); ++i)
            {
                vkDestroyFramebuffer(m_device, m_framebuffers[i], nullptr);
                vkDestroyRenderPass(m_device, m_render_passes[i], nullptr);
            }
            m_framebuffers.Clear();
            m_render_passes.Clear();

            m_depth_texture.reset();

            vkDestroyPipelineCache(m_device, m_pipeline_cache, nullptr);

            for (int i = 0; i < m_swapchain_image_resources.Size(); ++i)
            {
                vkFreeCommandBuffers(m_device, m_graphics_cmd_pool, 1, &m_swapchain_image_resources[i].cmd);
            }
            vkFreeCommandBuffers(m_device, m_graphics_cmd_pool, 1, &m_image_cmd);
            vkDestroyCommandPool(m_device, m_graphics_cmd_pool, nullptr);

            for (int i = 0; i < m_swapchain_image_resources.Size(); ++i)
            {
                vkDestroyImageView(m_device, m_swapchain_image_resources[i].image_view, nullptr);
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

                m_swapchain_image_resources[i].image = swapchain_images[i];

                err = vkCreateImageView(m_device, &view_info, nullptr, &m_swapchain_image_resources[i].image_view);
                assert(!err);

                m_swapchain_image_resources[i].format = m_surface_format.format;
                m_swapchain_image_resources[i].width = swapchain_size.width;
                m_swapchain_image_resources[i].height = swapchain_size.height;
            }
        }

        void CreateCommandBuffer()
        {
            VkCommandPoolCreateInfo pool_info;
            Memory::Zero(&pool_info, sizeof(pool_info));
            pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            pool_info.pNext = nullptr;
            pool_info.flags = 0;
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

        void CreatePipelineCache()
        {
            VkPipelineCacheCreateInfo pipeline_cache;
            Memory::Zero(&pipeline_cache, sizeof(pipeline_cache));
            pipeline_cache.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
            
            VkResult err;
            err = vkCreatePipelineCache(m_device, &pipeline_cache, nullptr, &m_pipeline_cache);
            assert(!err);
        }

        void CreateRenderPass(
            VkImageView color_image_view,
            VkFormat color_format,
            VkImageView depth_image_view,
            VkFormat depth_format,
            int image_width,
            int image_height,
            CameraClearFlags clear_flag,
            bool present,
            VkRenderPass* render_pass,
            VkFramebuffer* framebuffer)
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

            if (depth_image_view != nullptr)
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
            if (depth_image_view != nullptr)
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

            // create framebuffer
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
            fb_info.renderPass = *render_pass;
            fb_info.attachmentCount = attachments_view.Size();
            fb_info.pAttachments = &attachments_view[0];
            fb_info.width = (uint32_t) image_width;
            fb_info.height = (uint32_t) image_height;
            fb_info.layers = 1;

            err = vkCreateFramebuffer(m_device, &fb_info, nullptr, framebuffer);
            assert(!err);
        }

        void CreateRenderPasses()
        {
            m_render_passes.Resize(m_swapchain_image_resources.Size());
            m_framebuffers.Resize(m_swapchain_image_resources.Size());

            for (int i = 0; i < m_swapchain_image_resources.Size(); ++i)
            {
                this->CreateRenderPass(
                    m_swapchain_image_resources[i].image_view,
                    m_swapchain_image_resources[i].format,
                    nullptr,
                    VK_FORMAT_UNDEFINED,
                    m_swapchain_image_resources[i].width,
                    m_swapchain_image_resources[i].height,
                    CameraClearFlags::Color,
                    true,
                    &m_render_passes[i],
                    &m_framebuffers[i]
                    );
            }
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
}
