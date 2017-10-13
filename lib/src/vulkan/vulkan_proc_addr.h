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

#pragma once

#include "vulkan_include.h"

namespace Viry3D
{
	void get_instance_proc_addrs(VkInstance instance);
	void get_device_proc_addrs(VkInstance instance, VkDevice device);

	extern PFN_vkCreateDebugReportCallbackEXT fpCreateDebugReportCallbackEXT;
	extern PFN_vkDestroyDebugReportCallbackEXT fpDestroyDebugReportCallbackEXT;
	extern PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR;
	extern PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
	extern PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR;
	extern PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR;
	extern PFN_vkGetDeviceProcAddr fpGetDeviceProcAddr;
	extern PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
	extern PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
	extern PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
	extern PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
	extern PFN_vkQueuePresentKHR fpQueuePresentKHR;

#if VR_ANDROID
	extern PFN_vkCreateAndroidSurfaceKHR fpCreateAndroidSurfaceKHR;
#endif
}
