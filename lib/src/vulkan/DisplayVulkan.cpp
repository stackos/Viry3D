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

#include "DisplayVulkan.h"
#include "vulkan_check.h"
#include "vulkan_proc_addr.h"
#include "Application.h"
#include "Debug.h"
#include "memory/Memory.h"
#include "graphics/VertexBuffer.h"
#include "graphics/IndexBuffer.h"
#include "graphics/Graphics.h"
#include "thread/Thread.h"
#include "Profiler.h"

#if VR_VULKAN

#if VR_ANDROID
#include "android/jni.h"
#endif

#define VSYNC 0

namespace Viry3D
{
	DisplayVulkan::DisplayVulkan():
		m_instance(NULL),
		m_debug_callback(VK_NULL_HANDLE),
		m_gpu(NULL),
		m_surface(VK_NULL_HANDLE),
		m_graphics_queue_index(0),
		m_device(NULL),
		m_queue(NULL),
		m_swap_buffer_index(0),
		m_image_acquired_semaphore(VK_NULL_HANDLE),
		m_current_draw_cmd(NULL),
		m_swapchain(VK_NULL_HANDLE),
		m_cmd_pool(VK_NULL_HANDLE),
		m_image_cmd_pool(VK_NULL_HANDLE),
		m_image_cmd_buffer(NULL),
		m_pipeline_cache(VK_NULL_HANDLE)
	{
		Memory::Zero(&m_surface_format, sizeof(m_surface_format));
		Memory::Zero(&m_memory_properties, sizeof(m_memory_properties));
	}

	void DisplayVulkan::Deinit()
	{
		DestroySizeDependentResources();

		if (m_image_acquired_semaphore != VK_NULL_HANDLE)
		{
			vkDestroySemaphore(m_device, m_image_acquired_semaphore, NULL);
		}
		for (auto i : m_draw_complete_semaphores)
		{
			vkDestroySemaphore(m_device, i, NULL);
		}
		m_draw_complete_semaphores.Clear();

		fpDestroySwapchainKHR(m_device, m_swapchain, NULL);
		vkDestroyDevice(m_device, NULL);

		vkDestroySurfaceKHR(m_instance, m_surface, NULL);
		fpDestroyDebugReportCallbackEXT(m_instance, m_debug_callback, NULL);
		vkDestroyInstance(m_instance, NULL);
	}

	void DisplayVulkan::DestroySizeDependentResources()
	{
		vkDestroyPipelineCache(m_device, m_pipeline_cache, NULL);
		m_depth_texture.reset();
		for (int i = 0; i < m_swapchain_buffers.Size(); i++)
		{
			vkDestroyImageView(m_device, m_swapchain_buffers[i].image_view, NULL);
		}
		m_swapchain_buffers.Clear();
		vkFreeCommandBuffers(m_device, m_image_cmd_pool, 1, &m_image_cmd_buffer);
		vkDestroyCommandPool(m_device, m_image_cmd_pool, NULL);
		vkDestroyCommandPool(m_device, m_cmd_pool, NULL);
	}

	void DisplayVulkan::Init(int width, int height, int fps)
	{
#if VR_WINDOWS
		DisplayWindows::Init(width, height, fps);
#elif VR_ANDROID
		DisplayAndroid::Init(width, height, fps);
		int success = InitVulkan();
		Log("android vulkan so load success: %s", success ? "true" : "false");
#endif

		this->CreateInstance();
		get_instance_proc_addrs(m_instance);

		this->CreateDebugReportCallback();

		this->GetGPU();
		this->CreateSurface();

		m_graphics_queue_index = check_queue(m_gpu, m_surface);
		m_surface_format = check_surface_format(m_gpu, m_surface);

		this->CreateDevice();
		get_device_proc_addrs(m_instance, m_device);

		vkGetDeviceQueue(m_device, m_graphics_queue_index, 0, &m_queue);
		vkGetPhysicalDeviceMemoryProperties(m_gpu, &m_memory_properties);
		vkGetPhysicalDeviceProperties(m_gpu, &m_device_properties);

		m_device_name = m_device_properties.deviceName;

		this->CreateSizeDependentResources();

		Log("display vulkan init success");
	}

	void DisplayVulkan::OnResize(int width, int height)
	{
		if (m_device == NULL)
		{
			return;
		}

		vkDeviceWaitIdle(m_device);

		m_width = width;
		m_height = height;

		DestroySizeDependentResources();

		fpDestroySwapchainKHR(m_device, m_swapchain, NULL);
		m_swapchain = VK_NULL_HANDLE;

		vkDestroySurfaceKHR(m_instance, m_surface, NULL);
		m_surface = VK_NULL_HANDLE;

		this->CreateSurface();

		m_graphics_queue_index = check_queue(m_gpu, m_surface);
		m_surface_format = check_surface_format(m_gpu, m_surface);

		vkGetDeviceQueue(m_device, m_graphics_queue_index, 0, &m_queue);

		this->CreateSizeDependentResources();
	}

	void DisplayVulkan::OnPause()
	{
		Log("DisplayVulkan::OnPause");

		vkDeviceWaitIdle(m_device);

		DestroySizeDependentResources();

		fpDestroySwapchainKHR(m_device, m_swapchain, NULL);
		m_swapchain = VK_NULL_HANDLE;

		vkDestroySurfaceKHR(m_instance, m_surface, NULL);
		m_surface = VK_NULL_HANDLE;
	}

	void DisplayVulkan::OnResume()
	{
		Log("DisplayVulkan::OnResume");

		this->CreateSurface();

		m_graphics_queue_index = check_queue(m_gpu, m_surface);
		m_surface_format = check_surface_format(m_gpu, m_surface);

		vkGetDeviceQueue(m_device, m_graphics_queue_index, 0, &m_queue);

		this->CreateSizeDependentResources();
	}

	void DisplayVulkan::CreateInstance()
	{
		VkResult err;

		String name = Application::Current()->GetName();

		VkApplicationInfo app = {
			VK_STRUCTURE_TYPE_APPLICATION_INFO,
			NULL,
			name.CString(),
			0,
			name.CString(),
			0,
			VK_API_VERSION_1_0,
		};

		const char* extension_names[64] = { 0 };
		uint32_t extension_count = check_instance_extensions(extension_names);

#if VR_WINDOWS
		uint32_t instance_layer_count = 1;
		const char* instance_validation_layers[] = { "VK_LAYER_LUNARG_standard_validation" };

		VkInstanceCreateInfo inst = {
			VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			NULL,
			0,
			&app,
			instance_layer_count,
			instance_validation_layers,
			extension_count,
			(const char* const*) extension_names,
		};
#elif VR_ANDROID
		uint32_t instance_layer_count = 7;
		const char* instance_validation_layers[] = {
			"VK_LAYER_GOOGLE_threading",
			"VK_LAYER_LUNARG_parameter_validation",
			"VK_LAYER_LUNARG_object_tracker",
			"VK_LAYER_LUNARG_core_validation",
			"VK_LAYER_LUNARG_image",
			"VK_LAYER_LUNARG_swapchain",
			"VK_LAYER_GOOGLE_unique_objects"
		};

		VkInstanceCreateInfo inst = {
			VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			NULL,
			0,
			&app,
			instance_layer_count,
			instance_validation_layers,
			extension_count,
			(const char* const*) extension_names,
		};
#endif

		err = vkCreateInstance(&inst, NULL, &m_instance);
		assert(!err);
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL
		debug_func(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType,
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

	void DisplayVulkan::CreateDebugReportCallback()
	{
		VkDebugReportCallbackCreateInfoEXT create_info;
		create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
		create_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		create_info.pfnCallback = debug_func;
		create_info.pUserData = NULL;
		create_info.pNext = NULL;
		VkResult err = fpCreateDebugReportCallbackEXT(m_instance, &create_info, NULL, &m_debug_callback);
		assert(!err);
	}

	void DisplayVulkan::GetGPU()
	{
		VkResult err;

		uint32_t gpu_count;
		err = vkEnumeratePhysicalDevices(m_instance, &gpu_count, NULL);
		assert(!err && gpu_count > 0);

		VkPhysicalDevice* physical_devices = Memory::Alloc<VkPhysicalDevice>(sizeof(VkPhysicalDevice) * gpu_count);
		err = vkEnumeratePhysicalDevices(m_instance, &gpu_count, physical_devices);
		assert(!err);

		m_gpu = physical_devices[0];

		Memory::Free(physical_devices);
	}

	void DisplayVulkan::CreateSurface()
	{
		VkResult err;

#if VR_WINDOWS
		VkWin32SurfaceCreateInfoKHR create;
		create.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		create.pNext = NULL;
		create.flags = 0;
		create.hinstance = GetModuleHandle(NULL);
		create.hwnd = m_window;

		err = vkCreateWin32SurfaceKHR(m_instance, &create, NULL, &m_surface);
		assert(!err);
#elif VR_ANDROID
		VkAndroidSurfaceCreateInfoKHR create;
		create.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
		create.pNext = NULL;
		create.flags = 0;
		create.window = (ANativeWindow*) get_native_window();
		err = fpCreateAndroidSurfaceKHR(m_instance, &create, NULL, &m_surface);
		assert(!err);
#endif
	}

	void DisplayVulkan::CreateDevice()
	{
		VkResult err;

		float queue_priorities[1] = { 1.0f };
		VkDeviceQueueCreateInfo queue = {
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			NULL,
			0,
			m_graphics_queue_index,
			1,
			queue_priorities
		};

		uint32_t extension_count = 1;
		const char* extension_names[] = { "VK_KHR_swapchain" };

		VkDeviceCreateInfo device = {
			VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			NULL,
			0,
			1,
			&queue,
			0,
			NULL,
			extension_count,
			extension_names,
			NULL,
		};

		err = vkCreateDevice(m_gpu, &device, NULL, &m_device);
		assert(!err);
	}

	void DisplayVulkan::CreateSizeDependentResources()
	{
		this->CreateSwapchain();
		this->CreateCommandPool();
		this->CreateSwapchainBuffers();
		this->CreateImageCommandBuffer();

		m_depth_texture = RenderTexture::Create(
			m_width, m_height,
			RenderTextureFormat::Depth,
			DepthBuffer::Depth_24_Stencil_8,
			FilterMode::Point);

		this->CreatePipelineCache();

		m_swap_buffer_index = 0;
	}

	void DisplayVulkan::CreateSwapchain()
	{
		VkResult err;

		VkSurfaceCapabilitiesKHR surf_capabilities;
		err = fpGetPhysicalDeviceSurfaceCapabilitiesKHR(m_gpu, m_surface, &surf_capabilities);
		assert(!err);

		VkExtent2D extent;
		if (surf_capabilities.currentExtent.width == (uint32_t) -1)
		{
			extent.width = m_width;
			extent.height = m_height;
		}
		else
		{
			extent = surf_capabilities.currentExtent;
			m_width = extent.width;
			m_height = extent.height;
		}

		uint32_t image_count = surf_capabilities.minImageCount;
		m_swapchain_buffers.Resize(image_count);
		Memory::Zero(&m_swapchain_buffers[0], m_swapchain_buffers.SizeInBytes());

		VkSurfaceTransformFlagsKHR transform;
		if (surf_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		{
			transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		}
		else
		{
			transform = surf_capabilities.currentTransform;
		}

		uint32_t present_mode_count;
		err = fpGetPhysicalDeviceSurfacePresentModesKHR(m_gpu, m_surface, &present_mode_count, NULL);
		assert(!err);

		VkPresentModeKHR *present_modes = Memory::Alloc<VkPresentModeKHR>(sizeof(VkPresentModeKHR) * present_mode_count);
		err = fpGetPhysicalDeviceSurfacePresentModesKHR(m_gpu, m_surface, &present_mode_count, present_modes);
		assert(!err);

#if VSYNC || VR_ANDROID
		VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
#else
		VkPresentModeKHR present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
#endif

		Memory::Free(present_modes);

		VkSwapchainKHR old_swapchain = m_swapchain;
		VkSwapchainCreateInfoKHR swapchain_info = {
			VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			NULL,
			0,
			m_surface,
			image_count,
			m_surface_format.format,
            m_surface_format.colorSpace,
			extent,
			1,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			NULL,
			(VkSurfaceTransformFlagBitsKHR) transform,
#if VR_WINDOWS
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
#else
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
#endif
			present_mode,
            false,
			old_swapchain
		};
		err = fpCreateSwapchainKHR(m_device, &swapchain_info, NULL, &m_swapchain);
		assert(!err);

		if (old_swapchain != VK_NULL_HANDLE)
		{
			fpDestroySwapchainKHR(m_device, old_swapchain, NULL);
		}
	}

	void DisplayVulkan::CreateSwapchainBuffers()
	{
		VkResult err;

		uint32_t image_count;
		err = fpGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, NULL);
		assert(!err);
		assert(image_count >= (uint32_t) m_swapchain_buffers.Size());

		VkImage *images = Memory::Alloc<VkImage>(sizeof(VkImage) * image_count);
		err = fpGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, images);
		assert(!err);

		for (int i = 0; i < m_swapchain_buffers.Size(); i++)
		{
			m_swapchain_buffers[i].image = images[i];

			VkImageViewCreateInfo color_image_view = {
				VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				NULL,
				0,
				images[i],
				VK_IMAGE_VIEW_TYPE_2D,
				m_surface_format.format,
				{
					VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
					VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A,
				},
				{
					VK_IMAGE_ASPECT_COLOR_BIT,
					0, 1, 0, 1
				}
			};

			err = vkCreateImageView(m_device, &color_image_view, NULL, &m_swapchain_buffers[i].image_view);
			assert(!err);
		}

		Memory::Free(images);
	}

	void DisplayVulkan::CreateCommandPool()
	{
		VkResult err;

		VkCommandPoolCreateInfo cmd_pool_info = {
			VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			NULL,
			VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			m_graphics_queue_index,
		};
		err = vkCreateCommandPool(m_device, &cmd_pool_info, NULL, &m_cmd_pool);
		assert(!err);

		err = vkCreateCommandPool(m_device, &cmd_pool_info, NULL, &m_image_cmd_pool);
		assert(!err);
	}

	void DisplayVulkan::CreateImageCommandBuffer()
	{
		VkResult err;

		VkCommandBufferAllocateInfo cmd_info = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			NULL,
			m_image_cmd_pool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			1,
		};

		err = vkAllocateCommandBuffers(m_device, &cmd_info, &m_image_cmd_buffer);
		assert(!err);
	}

	void DisplayVulkan::BeginImageCommandBuffer()
	{
		VkResult err;

		m_mutex.lock();

		VkCommandBufferBeginInfo cmd_buf_info;
		cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmd_buf_info.pNext = NULL;
		cmd_buf_info.flags = 0;
		cmd_buf_info.pInheritanceInfo = NULL;

		err = vkBeginCommandBuffer(m_image_cmd_buffer, &cmd_buf_info);
		assert(!err);
	}

	void DisplayVulkan::EndImageCommandBuffer()
	{
		VkResult err;

		err = vkEndCommandBuffer(m_image_cmd_buffer);
		assert(!err);

		VkCommandBuffer cmd_bufs[] = { m_image_cmd_buffer };
		VkFence nullFence = VK_NULL_HANDLE;

		VkSubmitInfo submit_info;
		Memory::Zero(&submit_info, sizeof(submit_info));
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pNext = NULL;
		submit_info.waitSemaphoreCount = 0;
		submit_info.pWaitSemaphores = NULL;
		submit_info.pWaitDstStageMask = NULL;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = cmd_bufs;
		submit_info.signalSemaphoreCount = 0;
		submit_info.pSignalSemaphores = NULL;

		err = vkQueueSubmit(m_queue, 1, &submit_info, nullFence);
		assert(!err);

		err = vkQueueWaitIdle(m_queue);
		assert(!err);

		m_mutex.unlock();
	}

	void DisplayVulkan::SetImageLayout(
		VkImage image,
		VkImageAspectFlags aspect_mask,
		VkImageLayout old_image_layout,
		VkImageLayout new_image_layout,
		VkAccessFlagBits src_access_mask,
		VkImageSubresourceRange* subresource_range)
	{
		VkImageMemoryBarrier image_memory_barrier;
		Memory::Zero(&image_memory_barrier, sizeof(image_memory_barrier));
		image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		image_memory_barrier.pNext = NULL;
		image_memory_barrier.srcAccessMask = src_access_mask;
		image_memory_barrier.dstAccessMask = 0;
		image_memory_barrier.oldLayout = old_image_layout;
		image_memory_barrier.newLayout = new_image_layout;
		image_memory_barrier.image = image;

		if (subresource_range == NULL)
		{
			image_memory_barrier.subresourceRange = { aspect_mask, 0, 1, 0, 1 };
		}
		else
		{
			image_memory_barrier.subresourceRange = *subresource_range;
		}

		if (new_image_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		}

		if (new_image_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			/* Make sure anything that was copying from this image has completed */
			image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		}

		if (new_image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			image_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}

		if (new_image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			image_memory_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}

		if (new_image_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			/* Make sure any Copy or CPU writes to image are flushed */
			image_memory_barrier.dstAccessMask =
				VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
		}

		VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		VkPipelineStageFlags dest_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

		vkCmdPipelineBarrier(m_image_cmd_buffer, src_stages, dest_stages, 0, 0, NULL, 0, NULL, 1, &image_memory_barrier);
	}

	void DisplayVulkan::CreatePipelineCache()
	{
		VkPipelineCacheCreateInfo pipeline_cache;
		Memory::Zero(&pipeline_cache, sizeof(pipeline_cache));
		pipeline_cache.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

		VkResult err;
		err = vkCreatePipelineCache(m_device, &pipeline_cache, NULL, &m_pipeline_cache);
		assert(!err);
	}

	bool DisplayVulkan::CheckMemoryType(
		uint32_t type_bits,
		VkFlags requirements_mask,
		uint32_t* type_index)
	{
		// Search memtypes to find first index with those properties
		for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++)
		{
			if ((type_bits & 1) == 1)
			{
				// Type is available, does it match user properties?
				if ((m_memory_properties.memoryTypes[i].propertyFlags &
					requirements_mask) == requirements_mask)
				{
					*type_index = i;
					return true;
				}
			}
			type_bits >>= 1;
		}
		// No memory types matched, return failure
		return false;
	}

	VkShaderModule DisplayVulkan::CreateShaderModule(void *spv_bytes, int size)
	{
		VkShaderModuleCreateInfo moduleCreateInfo;
		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCreateInfo.pNext = NULL;
		moduleCreateInfo.codeSize = size;
		moduleCreateInfo.pCode = (const uint32_t *) spv_bytes;
		moduleCreateInfo.flags = 0;

		VkShaderModule module;
		VkResult err = vkCreateShaderModule(m_device, &moduleCreateInfo, NULL, &module);
		assert(!err);

		return module;
	}

	void DisplayVulkan::BeginFrame()
	{
		VkResult err;

		Profiler::SampleBegin("DisplayVulkan::BeginFrame");

		m_mutex.lock();

		err = vkQueueWaitIdle(m_queue);
		assert(!err);

		err = vkDeviceWaitIdle(m_device);
		assert(!err);

		if (m_image_acquired_semaphore != VK_NULL_HANDLE)
		{
			vkDestroySemaphore(m_device, m_image_acquired_semaphore, NULL);
		}
		for (auto i : m_draw_complete_semaphores)
		{
			vkDestroySemaphore(m_device, i, NULL);
		}
		m_draw_complete_semaphores.Clear();

		VkSemaphoreCreateInfo semaphore = {
			VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			NULL,
			0,
		};
		err = vkCreateSemaphore(m_device, &semaphore, NULL, &m_image_acquired_semaphore);
		assert(!err);

		uint32_t swap_buffer_index;
		err = fpAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX,
			m_image_acquired_semaphore,
			(VkFence) 0,
			&swap_buffer_index);
		assert(!err);
		assert(m_swap_buffer_index == swap_buffer_index);

		m_mutex.unlock();

		Profiler::SampleEnd();
	}

	void DisplayVulkan::EndFrame()
	{
		VkResult err;

		Profiler::SampleBegin("DisplayVulkan::EndFrame");

		m_mutex.lock();

		err = vkQueueWaitIdle(m_queue);
		assert(!err);

		err = vkDeviceWaitIdle(m_device);
		assert(!err);

		VkPresentInfoKHR present;
		Memory::Zero(&present, sizeof(present));
		present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present.pNext = NULL;
		present.swapchainCount = 1;
		present.pSwapchains = &m_swapchain;
		present.pImageIndices = &m_swap_buffer_index;
		present.waitSemaphoreCount = 1;
		present.pWaitSemaphores = &m_draw_complete_semaphores[m_draw_complete_semaphores.Size() - 1];

		err = fpQueuePresentKHR(m_queue, &present);
		assert(!err);

		err = vkQueueWaitIdle(m_queue);
		assert(!err);

		err = vkDeviceWaitIdle(m_device);
		assert(!err);

		m_swap_buffer_index++;
		if (m_swap_buffer_index == m_swapchain_buffers.Size())
		{
			m_swap_buffer_index = 0;
		}

		m_mutex.unlock();

		Profiler::SampleEnd();
	}

	void DisplayVulkan::WaitQueueIdle()
	{
		vkQueueWaitIdle(m_queue);
	}

	void DisplayVulkan::BeginPrimaryCommandBuffer(VkCommandBuffer cmd)
	{
		m_current_draw_cmd = cmd;

		VkCommandBufferBeginInfo cmd_buf_info = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			NULL,
			0,
			NULL,
		};

		VkResult err = vkBeginCommandBuffer(cmd, &cmd_buf_info);
		assert(!err);
	}

	void DisplayVulkan::EndPrimaryCommandBuffer()
	{
		VkResult err = vkEndCommandBuffer(m_current_draw_cmd);
		assert(!err);

		m_current_draw_cmd = NULL;
	}

	void DisplayVulkan::SubmitQueue(VkCommandBuffer cmd)
	{
		m_mutex.lock();

		VkSemaphoreCreateInfo semaphore = {
			VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			NULL,
			0,
		};

		VkSemaphore draw_complete_semaphore;
		VkResult err = vkCreateSemaphore(m_device, &semaphore, NULL, &draw_complete_semaphore);
		assert(!err);

		VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

		VkSubmitInfo submit_info;
		Memory::Zero(&submit_info, sizeof(submit_info));
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pNext = NULL;
		submit_info.waitSemaphoreCount = 1;
		if (m_draw_complete_semaphores.Empty())
		{
			submit_info.pWaitSemaphores = &m_image_acquired_semaphore;
		}
		else
		{
			submit_info.pWaitSemaphores = &m_draw_complete_semaphores[m_draw_complete_semaphores.Size() - 1];
		}
		submit_info.pWaitDstStageMask = &pipe_stage_flags;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &cmd;
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &draw_complete_semaphore;

		err = vkQueueSubmit(m_queue, 1, &submit_info, VK_NULL_HANDLE);
		assert(!err);

		err = vkQueueWaitIdle(m_queue);
		assert(!err);

		err = vkDeviceWaitIdle(m_device);
		assert(!err);

		m_draw_complete_semaphores.Add(draw_complete_semaphore);

		m_mutex.unlock();
	}

	void DisplayVulkan::BindVertexBuffer(const VertexBuffer* buffer)
	{
		VkBuffer buf = buffer->GetBuffer();
		VkDeviceSize offsets[1] = { 0 };
		VkCommandBuffer cmd = GetCurrentDrawCommand();

		vkCmdBindVertexBuffers(cmd, 0, 1, &buf, offsets);
	}

	void DisplayVulkan::BindIndexBuffer(const IndexBuffer* buffer, IndexType index_type)
	{
		VkIndexType type;
		VkCommandBuffer cmd = GetCurrentDrawCommand();

		if (index_type == IndexType::UnsignedShort)
		{
			type = VK_INDEX_TYPE_UINT16;
		}
		else if (index_type == IndexType::UnsignedInt)
		{
			type = VK_INDEX_TYPE_UINT32;
		}

		vkCmdBindIndexBuffer(cmd, buffer->GetBuffer(), 0, type);
	}

	void DisplayVulkan::DrawIndexed(int start, int count, IndexType index_type)
	{
		VkCommandBuffer cmd = GetCurrentDrawCommand();

		vkCmdDrawIndexed(cmd, count, 1, start, 0, 0);

		Graphics::draw_call++;
	}
}

#endif
