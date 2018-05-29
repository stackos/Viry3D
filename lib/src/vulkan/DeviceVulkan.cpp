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

namespace Viry3D
{
    class DeviceVulkanPrivate
    {
    public:
        DeviceVulkan * m_public;
        Vector<char*> m_enabled_layers;
        Vector<char*> m_extension_names;
        VkInstance m_instance;

        DeviceVulkanPrivate(DeviceVulkan* device):
            m_public(device),
            m_instance(VK_NULL_HANDLE)
        {
            
        }

        ~DeviceVulkanPrivate()
        {
            StringVectorClear(m_enabled_layers);
            StringVectorClear(m_extension_names);
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
            VkResult err = vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
            assert(!err);
            assert(instance_layer_count > 0);

            VkLayerProperties* instance_layers = Memory::Alloc<VkLayerProperties>(sizeof(VkLayerProperties) * instance_layer_count);
            err = vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers);
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
                instance_layers,
                instance_layer_count);
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
                    instance_layers,
                    instance_layer_count);
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
                    instance_layers,
                    instance_layer_count);
            }

            if (validation_found)
            {
                for (int i = 0; i < instance_validation_layer_count; ++i)
                {
                    StringVectorAdd(m_enabled_layers, instance_validation_layers[i]);
                }
            }

            Memory::Free(instance_layers);
        }

        void CheckInstanceExtensions()
        {
            uint32_t instance_extension_count;
            VkResult err = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, NULL);
            assert(!err);
            assert(instance_extension_count > 0);

            VkExtensionProperties* instance_extensions = Memory::Alloc<VkExtensionProperties>(sizeof(VkExtensionProperties) * instance_extension_count);
            err = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, instance_extensions);
            assert(!err);

            bool surface_ext_found = false;
            bool platform_surface_ext_found = false;

            for (uint32_t i = 0; i < instance_extension_count; ++i)
            {
                if (strcmp(VK_KHR_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName) == 0)
                {
                    surface_ext_found = true;
                    StringVectorAdd(m_extension_names, VK_KHR_SURFACE_EXTENSION_NAME);
                }
#if defined(VK_USE_PLATFORM_WIN32_KHR)
                if (strcmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName) == 0)
                {
                    platform_surface_ext_found = true;
                    StringVectorAdd(m_extension_names, VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
                }
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
                if (strcmp(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName) == 0)
                {
                    platform_surface_ext_found = true;
                    StringVectorAdd(m_extension_names, VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
                }
#elif defined(VK_USE_PLATFORM_IOS_MVK)
                if (strcmp(VK_MVK_IOS_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName) == 0)
                {
                    platform_surface_ext_found = true;
                    StringVectorAdd(m_extension_names, VK_MVK_IOS_SURFACE_EXTENSION_NAME);
                }
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
                if (strcmp(VK_MVK_MACOS_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName) == 0)
                {
                    platform_surface_ext_found = true;
                    StringVectorAdd(m_extension_names, VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
                }
#endif
                if (strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, instance_extensions[i].extensionName) == 0)
                {
                    platform_surface_ext_found = true;
                    StringVectorAdd(m_extension_names, VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
                }
                if (strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, instance_extensions[i].extensionName) == 0)
                {
                    platform_surface_ext_found = true;
                    StringVectorAdd(m_extension_names, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                }
            }

            assert(surface_ext_found);
            assert(platform_surface_ext_found);

            Memory::Free(instance_extensions);
        }

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
            VkDebugUtilsMessageTypeFlagsEXT message_type,
            const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
            void* user_data)
        {
            Log("%s", callback_data->pMessage);
            return false;
        }

        void CreateInstance()
        {
            VkDebugUtilsMessengerCreateInfoEXT dbg_messenger_create_info;
            dbg_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            dbg_messenger_create_info.pNext = NULL;
            dbg_messenger_create_info.flags = 0;
            dbg_messenger_create_info.messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            dbg_messenger_create_info.messageType =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            dbg_messenger_create_info.pfnUserCallback = DebugMessengerCallback;
            dbg_messenger_create_info.pUserData = this;

            String name = Application::Current()->GetName();

            VkApplicationInfo app_info;
            app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            app_info.pNext = NULL;
            app_info.pApplicationName = name.CString();
            app_info.applicationVersion = 0;
            app_info.pEngineName = name.CString();
            app_info.engineVersion = 0;
            app_info.apiVersion = VK_API_VERSION_1_0;

            VkInstanceCreateInfo inst_info;
            inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            inst_info.pNext = &dbg_messenger_create_info;
            inst_info.pApplicationInfo = &app_info;
            inst_info.enabledLayerCount = m_enabled_layers.Size();
            inst_info.ppEnabledLayerNames = &m_enabled_layers[0];
            inst_info.enabledExtensionCount = m_extension_names.Size();
            inst_info.ppEnabledExtensionNames = &m_extension_names[0];

            VkResult err = vkCreateInstance(&inst_info, NULL, &m_instance);
            assert(!err);
        }
    };

    DeviceVulkan::DeviceVulkan():
        m_private(new DeviceVulkanPrivate(this))
    {
    
    }

    DeviceVulkan::~DeviceVulkan()
    {
        delete m_private;
    }

    void DeviceVulkan::Init(int width, int height)
    {
        m_private->CheckInstanceLayers();
        m_private->CheckInstanceExtensions();
        m_private->CreateInstance();
    }

    void DeviceVulkan::Deinit()
    {
    
    }
}
