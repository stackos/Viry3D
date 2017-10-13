/*
* Viry3D
* Copyright 2014-2017 by Stack - stackos@qq.com
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

#include "vulkan_check.h"
#include "vulkan_proc_addr.h"
#include "memory/Memory.h"
#include <assert.h>

namespace Viry3D
{
	uint32_t check_instance_extensions(const char** extension_names)
	{
		uint32_t extension_count = 0;

		bool surface_ext_found = false;
		bool platform_surface_ext_found = false;
		VkResult err;

		uint32_t instance_extension_count = 0;
		err = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, NULL);
		assert(!err);

		if (instance_extension_count > 0)
		{
			VkExtensionProperties* instance_extensions = Memory::Alloc<VkExtensionProperties>(sizeof(VkExtensionProperties) * instance_extension_count);
			err = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, instance_extensions);
			assert(!err);

			for (uint32_t i = 0; i < instance_extension_count; i++)
			{
				if (!strcmp(VK_KHR_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName))
				{
					surface_ext_found = true;
					extension_names[extension_count++] = VK_KHR_SURFACE_EXTENSION_NAME;
				}

#if VK_USE_PLATFORM_WIN32_KHR
				if (!strcmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName))
				{
					platform_surface_ext_found = true;
					extension_names[extension_count++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
				}
#endif

				if (!strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, instance_extensions[i].extensionName))
				{
					extension_names[extension_count++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
				}

				assert(extension_count < 64);
			}

			Memory::Free(instance_extensions);
		}

		assert(surface_ext_found);
		assert(platform_surface_ext_found);

		return extension_count;
	}

	uint32_t check_queue(VkPhysicalDevice gpu, VkSurfaceKHR surface)
	{
		uint32_t queue_count;
		VkQueueFamilyProperties *queue_props;

		{
			vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_count, NULL);
			assert(queue_count >= 1);

			queue_props = Memory::Alloc<VkQueueFamilyProperties>(queue_count * sizeof(VkQueueFamilyProperties));
			vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_count, queue_props);

			uint32_t gfx_queue_idx = 0;
			for (gfx_queue_idx = 0; gfx_queue_idx < queue_count; gfx_queue_idx++)
			{
				if (queue_props[gfx_queue_idx].queueFlags & VK_QUEUE_GRAPHICS_BIT)
					break;
			}
			assert(gfx_queue_idx < queue_count);
		}

		VkBool32 *supports_present = Memory::Alloc<VkBool32>(queue_count * sizeof(VkBool32));
		for (uint32_t i = 0; i < queue_count; i++)
		{
			fpGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &supports_present[i]);
		}

		uint32_t graphics_index = UINT32_MAX;
		uint32_t present_index = UINT32_MAX;
		for (uint32_t i = 0; i < queue_count; i++)
		{
			if ((queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
			{
				if (graphics_index == UINT32_MAX)
				{
					graphics_index = i;
				}

				if (supports_present[i] == VK_TRUE)
				{
					graphics_index = i;
					present_index = i;
					break;
				}
			}
		}
		if (present_index == UINT32_MAX)
		{
			for (uint32_t i = 0; i < queue_count; ++i)
			{
				if (supports_present[i] == VK_TRUE)
				{
					present_index = i;
					break;
				}
			}
		}

		Memory::Free(supports_present);
		Memory::Free(queue_props);

		assert(graphics_index != UINT32_MAX && present_index != UINT32_MAX);
		assert(graphics_index == present_index);

		return graphics_index;
	}

	VkSurfaceFormatKHR check_surface_format(VkPhysicalDevice gpu, VkSurfaceKHR surface)
	{
		VkResult err;

		uint32_t format_count;
		err = fpGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &format_count, NULL);
		assert(!err);

		VkSurfaceFormatKHR *formats = Memory::Alloc<VkSurfaceFormatKHR>(sizeof(VkSurfaceFormatKHR) * format_count);
		err = fpGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &format_count, formats);
		assert(!err);

		assert(format_count > 0);

		VkSurfaceFormatKHR surface_format;
		if (format_count == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
		{
			surface_format.format = VK_FORMAT_B8G8R8A8_UNORM;
		}
		else
		{
			surface_format.format = formats[0].format;
		}
		surface_format.colorSpace = formats[0].colorSpace;

		Memory::Free(formats);

		return surface_format;
	}
}
