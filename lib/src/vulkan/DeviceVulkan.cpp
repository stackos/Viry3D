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

#include "DeviceVulkan.h"
#include "vulkan_include.h"
#include "container/Vector.h"
#include "string/String.h"
#include "memory/Memory.h"
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

namespace Viry3D
{
    class DeviceVulkanPrivate
    {
    public:
        DeviceVulkan* m_public;
        void* m_window;
        int m_width;
        int m_height;
        Vector<char*> m_enabled_layers;
        Vector<char*> m_instance_extension_names;
        Vector<char*> m_device_extension_names;
        VkInstance m_instance = nullptr;
        VkPhysicalDevice m_gpu = nullptr;
        VkSurfaceKHR m_surface = nullptr;
        VkDevice m_device = nullptr;
        VkQueue m_graphics_queue = nullptr;
        VkQueue m_present_queue = nullptr;
        VkPhysicalDeviceProperties m_gpu_properties;
        Vector<VkQueueFamilyProperties> m_queue_properties;
        VkPhysicalDeviceFeatures m_gpu_features;
        VkPhysicalDeviceMemoryProperties m_memory_properties;
        PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR = nullptr;
        PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
        PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR = nullptr;
        PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR = nullptr;
        PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR = nullptr;
        PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR = nullptr;
        PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR = nullptr;
        PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR = nullptr;
        PFN_vkQueuePresentKHR fpQueuePresentKHR = nullptr;
        int m_graphics_queue_index = -1;
        int m_present_queue_index = -1;
        bool m_separate_present_queue = false;
        VkSurfaceFormatKHR m_surface_format;
        VkFence m_fences[FRAME_LAG];
        VkSemaphore m_image_acquired_semaphores[FRAME_LAG];
        VkSemaphore m_draw_complete_semaphores[FRAME_LAG];
        VkSemaphore m_image_ownership_semaphores[FRAME_LAG];

        DeviceVulkanPrivate(DeviceVulkan* device, void* window, int width, int height):
            m_public(device),
            m_window(window),
            m_width(width),
            m_height(height)
        {
            Memory::Zero(m_fences, sizeof(m_fences));
            Memory::Zero(m_image_acquired_semaphores, sizeof(m_image_acquired_semaphores));
            Memory::Zero(m_draw_complete_semaphores, sizeof(m_draw_complete_semaphores));
            Memory::Zero(m_image_ownership_semaphores, sizeof(m_image_ownership_semaphores));
        }

        ~DeviceVulkanPrivate()
        {
            vkDeviceWaitIdle(m_device);

            for (int i = 0; i < FRAME_LAG; ++i)
            {
                vkWaitForFences(m_device, 1, &m_fences[i], VK_TRUE, UINT64_MAX);
                vkDestroyFence(m_device, m_fences[i], nullptr);
                vkDestroySemaphore(m_device, m_image_acquired_semaphores[i], nullptr);
                vkDestroySemaphore(m_device, m_draw_complete_semaphores[i], nullptr);
                if (m_separate_present_queue)
                {
                    vkDestroySemaphore(m_device, m_image_ownership_semaphores[i], nullptr);
                }
            }

            vkDestroyDevice(m_device, nullptr);
            vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
            m_gpu = nullptr;
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
            app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            app_info.pNext = nullptr;
            app_info.pApplicationName = name.CString();
            app_info.applicationVersion = 0;
            app_info.pEngineName = name.CString();
            app_info.engineVersion = 0;
            app_info.apiVersion = VK_API_VERSION_1_0;

            VkInstanceCreateInfo inst_info;
            inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            inst_info.pNext = nullptr;
            inst_info.pApplicationInfo = &app_info;
            inst_info.enabledLayerCount = m_enabled_layers.Size();
            inst_info.ppEnabledLayerNames = &m_enabled_layers[0];
            inst_info.enabledExtensionCount = m_instance_extension_names.Size();
            inst_info.ppEnabledExtensionNames = &m_instance_extension_names[0];

            VkResult err = vkCreateInstance(&inst_info, nullptr, &m_instance);
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
            
            GET_INSTANCE_PROC_ADDR(m_instance, GetPhysicalDeviceSurfaceSupportKHR);
            GET_INSTANCE_PROC_ADDR(m_instance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
            GET_INSTANCE_PROC_ADDR(m_instance, GetPhysicalDeviceSurfaceFormatsKHR);
            GET_INSTANCE_PROC_ADDR(m_instance, GetPhysicalDeviceSurfacePresentModesKHR);
            GET_INSTANCE_PROC_ADDR(m_instance, GetSwapchainImagesKHR);
        }

        void CreateDevice()
        {
            VkResult err;

#if defined(VK_USE_PLATFORM_WIN32_KHR)
            VkWin32SurfaceCreateInfoKHR create_info;
            create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
            create_info.pNext = nullptr;
            create_info.flags = 0;
            create_info.hinstance = GetModuleHandle(nullptr);
            create_info.hwnd = (HWND) m_window;
            
            err = vkCreateWin32SurfaceKHR(m_instance, &create_info, nullptr, &m_surface);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
            VkAndroidSurfaceCreateInfoKHR create_info;
            create_info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
            create_info.pNext = nullptr;
            create_info.flags = 0;
            create_info.window = (ANativeWindow*) m_window;

            err = vkCreateAndroidSurfaceKHR(m_instance, &create_info, nullptr, &m_surface);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
            VkIOSSurfaceCreateInfoMVK create_info;
            create_info.sType = VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK;
            create_info.pNext = nullptr;
            create_info.flags = 0;
            create_info.pView = m_window;

            err = vkCreateIOSSurfaceMVK(m_instance, &create_info, nullptr, &m_surface);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
            VkMacOSSurfaceCreateInfoMVK create_info;
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

            m_graphics_queue_index = graphics_queue_index;
            m_present_queue_index = present_queue_index;
            m_separate_present_queue = m_graphics_queue_index != m_present_queue_index;

            float queue_priorities[1] = { 0.0 };
            VkDeviceQueueCreateInfo queue_infos[2];
            queue_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_infos[0].pNext = nullptr;
            queue_infos[0].queueFamilyIndex = m_graphics_queue_index;
            queue_infos[0].queueCount = 1;
            queue_infos[0].pQueuePriorities = queue_priorities;
            queue_infos[0].flags = 0;

            VkDeviceCreateInfo device_info;
            device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            device_info.pNext = nullptr;
            device_info.queueCreateInfoCount = 1;
            device_info.pQueueCreateInfos = queue_infos;
            device_info.enabledLayerCount = m_enabled_layers.Size();
            device_info.ppEnabledLayerNames = &m_enabled_layers[0];
            device_info.enabledExtensionCount = m_device_extension_names.Size();
            device_info.ppEnabledExtensionNames = &m_device_extension_names[0];
            device_info.pEnabledFeatures = nullptr;

            if (m_separate_present_queue)
            {
                queue_infos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queue_infos[1].pNext = nullptr;
                queue_infos[1].queueFamilyIndex = m_present_queue_index;
                queue_infos[1].queueCount = 1;
                queue_infos[1].pQueuePriorities = queue_priorities;
                queue_infos[1].flags = 0;

                device_info.queueCreateInfoCount = 2;
            }

            err = vkCreateDevice(m_gpu, &device_info, nullptr, &m_device);
            assert(!err);

            GET_DEVICE_PROC_ADDR(m_device, CreateSwapchainKHR);
            GET_DEVICE_PROC_ADDR(m_device, DestroySwapchainKHR);
            GET_DEVICE_PROC_ADDR(m_device, GetSwapchainImagesKHR);
            GET_DEVICE_PROC_ADDR(m_device, AcquireNextImageKHR);
            GET_DEVICE_PROC_ADDR(m_device, QueuePresentKHR);

            vkGetDeviceQueue(m_device, m_graphics_queue_index, 0, &m_graphics_queue);

            if (m_separate_present_queue)
            {
                vkGetDeviceQueue(m_device, m_present_queue_index, 0, &m_present_queue);
            }
            else
            {
                m_present_queue = m_graphics_queue;
            }

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
            fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fence_info.pNext = nullptr;
            fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            VkSemaphoreCreateInfo semaphore_info;
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

                if (m_separate_present_queue)
                {
                    err = vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_image_ownership_semaphores[i]);
                    assert(!err);
                }
            }
        }
    };

    DeviceVulkan::DeviceVulkan(void* window, int width, int height):
        m_private(new DeviceVulkanPrivate(this, window, width, height))
    {
        m_private->CheckInstanceLayers();
        m_private->CheckInstanceExtensions();
        m_private->CreateInstance();
        m_private->InitPhysicalDevice();
        m_private->CreateDevice();
        m_private->CreateSemaphores();
        //demo_prepare
    }

    DeviceVulkan::~DeviceVulkan()
    {
        delete m_private;
    }
}
