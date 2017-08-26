#pragma once

#include "vulkan_include.h"

namespace Viry3D
{
	uint32_t check_instance_extensions(char** extension_names);
	uint32_t check_queue(VkPhysicalDevice gpu, VkSurfaceKHR surface);
	VkSurfaceFormatKHR check_surface_format(VkPhysicalDevice gpu, VkSurfaceKHR surface);
}