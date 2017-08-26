#pragma once

#include "vulkan_include.h"
#include "Object.h"

namespace Viry3D
{
	class ImageBuffer;

	class TextureVulkan : public Object
	{
	public:
		virtual ~TextureVulkan();

		VkFormat GetVkFormat() const { return m_format; }
		VkImageView GetImageView() const { return m_image_view; }
		VkImage GetImage() const { return m_image; }
		VkSampler GetSampler() const { return m_sampler; }
		void UpdateSampler();

	protected:
		TextureVulkan();
		void CreateColorRenderTexture();
		void CreateDepthRenderTexture();
		void CreateTexture2D();
		void UpdateTexture2D(int x, int y, int w, int h, const ByteBuffer& colors);

	private:
		void Create(VkImageTiling tiling,
			VkImageUsageFlags usage,
			VkFlags required_props,
			VkImageLayout init_layout,
			int mip_count);
		void CreateView(VkImageAspectFlags aspect_mask, VkComponentMapping components, int mip_count);
		void FillImageBuffer(const ByteBuffer& buffer, const Ref<ImageBuffer>& image_buffer);
		void CopyBufferImage(const Ref<ImageBuffer>& image_buffer, int x, int y, int w, int h);
		void GenMipmaps(int mip_count);
		void CreateSampler(int mip_count);
		int GetMipCount();

		VkFormat m_format;
		VkImage m_image;
		VkMemoryAllocateInfo m_memory_info;
		VkDeviceMemory m_memory;
		VkImageView m_image_view;
		VkSampler m_sampler;
		Ref<ImageBuffer> m_image_buffer;
		int m_image_buffer_size;
	};
}