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

#if VR_WINDOWS
#include "windows/DisplayWindows.h"
#endif

#include "vulkan_include.h"
#include "container/Vector.h"
#include "graphics/RenderTexture.h"
#include "graphics/IndexBuffer.h"
#include "graphics/VertexAttribute.h"
#include "thread/Thread.h"

namespace Viry3D
{
	class VertexBuffer;
	class IndexBuffer;
	class Shader;
	class ThreadPool;

#if VR_WINDOWS
	class DisplayVulkan: public DisplayWindows
	{
#endif

	public:
		DisplayVulkan();
		void Init(int width, int height, int fps);
		void Deinit();
		void OnResize(int width, int height);
		void OnPause() { }
		void OnResume() { }
		void BeginFrame();
		void EndFrame();
		void BeginPrimaryCommandBuffer(VkCommandBuffer cmd);
		void EndPrimaryCommandBuffer();
		void BindVertexBuffer(const VertexBuffer* buffer);
		void BindIndexBuffer(const IndexBuffer* buffer, IndexType index_type);
		void BindVertexArray(const Ref<Shader>& shader, int pass_index, const Vector<VertexAttributeOffset>& attrs) { }
		void DrawIndexed(int start, int count, IndexType index_type);
		void SubmitQueue(VkCommandBuffer cmd);

		void CreateSharedContext() { }
		void DestroySharedContext() { }
		void FlushContext() { }
		void SwapBuffers() { }

		VkDevice GetDevice() const { return m_device; }
		VkFormat GetSurfaceFormat() const { return m_surface_format.format; }
		const Ref<RenderTexture>& GetDepthTexture() const { return m_depth_texture; }
		VkImage GetSwapchainBufferImage(int index) const { return m_swapchain_buffers[index].image; }
		VkImageView GetSwapchainBufferImageView(int index) const { return m_swapchain_buffers[index].image_view; }
		VkCommandPool GetCommandPool() const { return m_cmd_pool; }
		VkCommandBuffer GetCurrentDrawCommand() const { return m_current_draw_cmd; }
		int GetSwapchainBufferCount() const { return m_swapchain_buffers.Size(); }
		int GetSwapBufferIndex() const { return m_swap_buffer_index; }
		VkPipelineCache GetPipelineCache() const { return m_pipeline_cache; }
		int GetMinUniformBufferOffsetAlignment() const { return (int) m_device_properties.limits.minUniformBufferOffsetAlignment; }
		const String& GetDeviceName() const { return m_device_name; }

		bool CheckMemoryType(uint32_t type_bits, VkFlags requirements_mask, uint32_t* type_index);
		void BeginImageCommandBuffer();
		void SetImageLayout(
			VkImage image,
			VkImageAspectFlags aspect_mask,
			VkImageLayout old_image_layout,
			VkImageLayout new_image_layout,
			VkAccessFlagBits src_access_mask,
			VkImageSubresourceRange* subresource_range = NULL);
		VkCommandBuffer GetImageCommandBuffer() const { return m_image_cmd_buffer; }
		void EndImageCommandBuffer();

	private:
		void CreateInstance();
		void CreateDebugReportCallback();
		void GetGPU();
		void CreateSurface();
		void CreateDevice();
		void CreateSizeDependentResources();
		void DestroySizeDependentResources();
		void CreateSwapchain();
		void CreateSwapchainBuffers();
		void CreateCommandPool();
		void CreateImageCommandBuffer();
		void CreatePipelineCache();

		VkShaderModule CreateShaderModule(void *spv_bytes, int size);

		struct SwapchainBuffer
		{
			VkImage image;
			VkImageView image_view;
		};

		struct ThreadData
		{
			VkCommandPool cmd_pool;
			Vector<VkCommandBuffer> cmd;
		};

		VkInstance m_instance;
		VkDebugReportCallbackEXT m_debug_callback;
		VkPhysicalDevice m_gpu;
		VkSurfaceKHR m_surface;
		uint32_t m_graphics_queue_index;
		VkDevice m_device;
		VkQueue m_queue;
		VkSurfaceFormatKHR m_surface_format;
		VkPhysicalDeviceMemoryProperties m_memory_properties;
		VkPhysicalDeviceProperties m_device_properties;

		Mutex m_mutex;
		uint32_t m_swap_buffer_index;
		VkSemaphore m_image_acquired_semaphore;
		Vector<VkSemaphore> m_draw_complete_semaphores;
		VkCommandBuffer m_current_draw_cmd;
		String m_device_name;

		// resources need recreate when window resize
		VkSwapchainKHR m_swapchain;
		Vector<SwapchainBuffer> m_swapchain_buffers;
		VkCommandPool m_cmd_pool;
		VkCommandPool m_image_cmd_pool;
		VkCommandBuffer m_image_cmd_buffer;
		Ref<RenderTexture> m_depth_texture;
		VkPipelineCache m_pipeline_cache;
	};
}
