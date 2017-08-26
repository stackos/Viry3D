#include "vulkan_proc_addr.h"

namespace Viry3D
{
#define GET_INSTANCE_PROC_ADDR(inst, entrypoint)									\
	{																				\
        fp##entrypoint =															\
            (PFN_vk##entrypoint) vkGetInstanceProcAddr(inst, "vk" #entrypoint);		\
	}

#define GET_DEVICE_PROC_ADDR(inst, dev, entrypoint)									\
	{																				\
        if(!fpGetDeviceProcAddr)                                                    \
            fpGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr) vkGetInstanceProcAddr(  \
                inst, "vkGetDeviceProcAddr");										\
        fp##entrypoint =															\
            (PFN_vk##entrypoint) fpGetDeviceProcAddr(dev, "vk" #entrypoint);		\
    }

	PFN_vkCreateDebugReportCallbackEXT fpCreateDebugReportCallbackEXT;
	PFN_vkDestroyDebugReportCallbackEXT fpDestroyDebugReportCallbackEXT;
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR;
	PFN_vkGetDeviceProcAddr fpGetDeviceProcAddr;
	PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
	PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
	PFN_vkQueuePresentKHR fpQueuePresentKHR;

	void get_instance_proc_addrs(VkInstance instance)
	{
		GET_INSTANCE_PROC_ADDR(instance, CreateDebugReportCallbackEXT);
		GET_INSTANCE_PROC_ADDR(instance, DestroyDebugReportCallbackEXT);

		GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfaceSupportKHR);
		GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
		GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfaceFormatsKHR);
		GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfacePresentModesKHR);
		GET_INSTANCE_PROC_ADDR(instance, GetSwapchainImagesKHR);
	}

	void get_device_proc_addrs(VkInstance instance, VkDevice device)
	{
		GET_DEVICE_PROC_ADDR(instance, device, CreateSwapchainKHR);
		GET_DEVICE_PROC_ADDR(instance, device, DestroySwapchainKHR);
		GET_DEVICE_PROC_ADDR(instance, device, GetSwapchainImagesKHR);
		GET_DEVICE_PROC_ADDR(instance, device, AcquireNextImageKHR);
		GET_DEVICE_PROC_ADDR(instance, device, QueuePresentKHR);
	}
}