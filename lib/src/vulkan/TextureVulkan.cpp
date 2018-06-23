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

#include "TextureVulkan.h"
#include "math/Mathf.h"
#include "memory/Memory.h"
#include "graphics/Graphics.h"
#include "graphics/RenderTexture.h"
#include "graphics/Texture2D.h"
#include "graphics/Cubemap.h"
#include "graphics/ImageBuffer.h"

#if VR_VULKAN

namespace Viry3D
{
	TextureVulkan::TextureVulkan():
		m_format(VK_FORMAT_UNDEFINED),
		m_image(VK_NULL_HANDLE),
		m_memory(VK_NULL_HANDLE),
		m_image_view(VK_NULL_HANDLE),
		m_sampler(VK_NULL_HANDLE)
	{
		SetName("TextureVulkan");
		Memory::Zero(&m_memory_info, sizeof(m_memory_info));
	}

	TextureVulkan::~TextureVulkan()
	{
		auto display = (DisplayVulkan*) Graphics::GetDisplay();
		auto device = display->GetDevice();

		if (m_sampler)
		{
			vkDestroySampler(device, m_sampler, NULL);
		}
		vkDestroyImageView(device, m_image_view, NULL);
		vkFreeMemory(device, m_memory, NULL);
		vkDestroyImage(device, m_image, NULL);
	}

	void TextureVulkan::CreateColorRenderTexture()
	{
		auto texture = (RenderTexture*) this;
		auto format = texture->GetFormat();

		switch (format)
		{
			case RenderTextureFormat::RGBA32:
				m_format = VK_FORMAT_R8G8B8A8_UNORM;
				break;
			case RenderTextureFormat::R8:
				m_format = VK_FORMAT_R8_UNORM;
				break;
			default:
				assert(!"color format not invalid");
				break;
		}

		this->Create(VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			false);

		this->CreateView(VK_IMAGE_ASPECT_COLOR_BIT, {
			VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY
			},
			false);

		this->CreateSampler();
	}

	void TextureVulkan::CreateDepthRenderTexture()
	{
		auto texture = (RenderTexture*) this;
		auto depth = texture->GetDepth();
		VkImageAspectFlags view_aspect = 0;

		if (depth == DepthBuffer::Depth_16)
		{
			m_format = VK_FORMAT_D16_UNORM;
			view_aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		}
		else if (depth == DepthBuffer::Depth_24)
		{
			m_format = VK_FORMAT_X8_D24_UNORM_PACK32;
			view_aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		}
		else if (depth == DepthBuffer::Depth_24_Stencil_8)
		{
			m_format = VK_FORMAT_D24_UNORM_S8_UINT;
			view_aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		else if (depth == DepthBuffer::Depth_32)
		{
			m_format = VK_FORMAT_D32_SFLOAT;
			view_aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		}
		else
		{
			assert(!"depth format not invalid");
		}

		this->Create(VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			false);

		this->CreateView(view_aspect, {
			VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY
			},
			false);

		this->CreateSampler();
	}

	void TextureVulkan::CreateTexture2D()
	{
		auto texture = (Texture2D*) this;
		int width = texture->GetWidth();
		int height = texture->GetHeight();
		auto format = texture->GetFormat();
		auto colors = texture->GetColors();
		int buffer_size;

		if (format == TextureFormat::RGBA32)
		{
			m_format = VK_FORMAT_R8G8B8A8_UNORM;
			buffer_size = width * height * 4;
		}
		else if (format == TextureFormat::RGB24)
		{
			m_format = VK_FORMAT_R8G8B8A8_UNORM;
			buffer_size = width * height * 4;

			int pixel_count = colors.Size() / 3;
			ByteBuffer temp(pixel_count * 4);
			for (int i = 0; i < pixel_count; i++)
			{
				temp[i * 4 + 0] = colors[i * 3 + 0];
				temp[i * 4 + 1] = colors[i * 3 + 1];
				temp[i * 4 + 2] = colors[i * 3 + 2];
				temp[i * 4 + 3] = 255;
			}
			colors = temp;
		}
		else if (format == TextureFormat::R8)
		{
			m_format = VK_FORMAT_R8_UNORM;
			buffer_size = width * height;
		}
		else
		{
			assert(!"texture format not implement");
		}

		m_image_buffers.Add(ImageBuffer::Create(buffer_size));

        this->Create(VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			false);

		this->CreateView(VK_IMAGE_ASPECT_COLOR_BIT,
			{ VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A },
			false);

		this->CreateSampler();

		this->FillImageBuffer(colors, m_image_buffers[0], 0, buffer_size);
		this->CopyBufferImageBegin();
		this->CopyBufferImage(m_image_buffers[0], 0, 0, width, height);
		this->CopyBufferImageEnd();

		this->GenerateMipmap();
	}

	void TextureVulkan::UpdateTexture2D(int x, int y, int w, int h, const ByteBuffer& colors)
	{
		auto texture = (Texture2D*) this;
		auto format = texture->GetFormat();
		int buffer_size;

		if (format == TextureFormat::RGBA32)
		{
			m_format = VK_FORMAT_R8G8B8A8_UNORM;
			buffer_size = w * h * 4;
		}
		else if (format == TextureFormat::RGB24)
		{
			m_format = VK_FORMAT_R8G8B8A8_UNORM;
			buffer_size = w * h * 4;

			int pixel_count = colors.Size() / 3;
			ByteBuffer temp(pixel_count * 4);
			for (int i = 0; i < pixel_count; i++)
			{
				temp[i * 4 + 0] = colors[i * 3 + 0];
				temp[i * 4 + 1] = colors[i * 3 + 1];
				temp[i * 4 + 2] = colors[i * 3 + 2];
				temp[i * 4 + 3] = 255;
			}
		}
		else if (format == TextureFormat::R8)
		{
			m_format = VK_FORMAT_R8_UNORM;
			buffer_size = w * h;
		}
		else
		{
			assert(!"texture format not implement");
		}

		if (m_image_buffers.Empty() || m_image_buffers[0]->GetSize() < buffer_size)
		{
			m_image_buffers.Clear();
			m_image_buffers.Add(ImageBuffer::Create(buffer_size));
		}

		this->FillImageBuffer(colors, m_image_buffers[0], 0, buffer_size);
		this->CopyBufferImageBegin();
		this->CopyBufferImage(m_image_buffers[0], x, y, w, h);
		this->CopyBufferImageEnd();
	}

	void TextureVulkan::Create(VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkFlags required_props,
		VkImageLayout init_layout,
		bool cubemap)
	{
		auto texture = (Texture*) this;
		uint32_t width = (uint32_t) texture->GetWidth();
		uint32_t height = (uint32_t) texture->GetHeight();
		int mip_count = texture->GetMipmapCount();
		auto display = (DisplayVulkan*) Graphics::GetDisplay();
		auto device = display->GetDevice();
		VkResult err;

		VkImageCreateInfo image;
		Memory::Zero(&image, sizeof(image));
		image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image.imageType = VK_IMAGE_TYPE_2D;
		image.samples = VK_SAMPLE_COUNT_1_BIT;
		image.format = m_format;
		image.extent = { width, height, 1 };
		image.mipLevels = mip_count;
		image.tiling = tiling;
		image.usage = usage;
		image.initialLayout = init_layout;
		image.flags = cubemap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
		image.arrayLayers = cubemap ? 6 : 1;

		err = vkCreateImage(device, &image, NULL, &m_image);
		assert(!err);

		VkMemoryRequirements mem_reqs;
		vkGetImageMemoryRequirements(device, m_image, &mem_reqs);
		assert(!err);

		m_memory_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		m_memory_info.pNext = NULL;
		m_memory_info.allocationSize = mem_reqs.size;
		m_memory_info.memoryTypeIndex = 0;

		bool pass = display->CheckMemoryType(
			mem_reqs.memoryTypeBits,
			required_props,
			&m_memory_info.memoryTypeIndex);
		assert(pass);

		err = vkAllocateMemory(device, &m_memory_info, NULL, &m_memory);
		assert(!err);

		err = vkBindImageMemory(device, m_image, m_memory, 0);
		assert(!err);
	}

	void TextureVulkan::CreateView(VkImageAspectFlags aspect_mask, VkComponentMapping components, bool cubemap)
	{
		auto texture = (Texture*) this;
		int mip_count = texture->GetMipmapCount();
		auto display = (DisplayVulkan*) Graphics::GetDisplay();
		auto device = display->GetDevice();
		VkResult err;

		VkImageViewCreateInfo view;
		Memory::Zero(&view, sizeof(view));
		view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view.viewType = cubemap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
		view.image = m_image;
		view.format = m_format;
		view.components = components;
		view.subresourceRange = {
			aspect_mask,
			0, (uint32_t) mip_count,
			0, (uint32_t) (cubemap ? 6 : 1)
		};

		err = vkCreateImageView(device, &view, NULL, &m_image_view);
		assert(!err);
	}

	void TextureVulkan::UpdateSampler()
	{
		auto display = (DisplayVulkan*) Graphics::GetDisplay();
		auto device = display->GetDevice();

		if (m_sampler)
		{
			vkDestroySampler(device, m_sampler, NULL);
			m_sampler = VK_NULL_HANDLE;
		}

		this->CreateSampler();
	}

	void TextureVulkan::CreateSampler()
	{
		auto texture = (Texture*) this;
		auto wrap_mode = texture->GetWrapMode();
		auto filter_mode = texture->GetFilterMode();
		auto mip_count = texture->GetMipmapCount();
		auto display = (DisplayVulkan*) Graphics::GetDisplay();
		auto device = display->GetDevice();
		VkResult err;
		VkFilter filter;
		VkSamplerAddressMode address_mode;

		switch (filter_mode)
		{
			case FilterMode::Point:
				filter = VK_FILTER_NEAREST;
				break;
			case FilterMode::Bilinear:
			case FilterMode::Trilinear:
				filter = VK_FILTER_LINEAR;
				break;
		}

		switch (wrap_mode)
		{
			case TextureWrapMode::Repeat:
				address_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				break;
			case TextureWrapMode::Clamp:
				address_mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				break;
		}

		VkSamplerCreateInfo sampler;
		Memory::Zero(&sampler, sizeof(sampler));
		sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler.pNext = NULL;
		sampler.magFilter = filter;
		sampler.minFilter = filter;
		sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler.addressModeU = address_mode;
		sampler.addressModeV = address_mode;
		sampler.addressModeW = address_mode;
		sampler.mipLodBias = 0.0f;
		sampler.anisotropyEnable = VK_FALSE;
		sampler.maxAnisotropy = 1;
		sampler.compareOp = VK_COMPARE_OP_NEVER;
		sampler.minLod = 0.0f;
		sampler.maxLod = mip_count > 1 ? (float) mip_count : 0.0f;
		sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		sampler.unnormalizedCoordinates = VK_FALSE;

		err = vkCreateSampler(device, &sampler, NULL, &m_sampler);
		assert(!err);
	}

	void TextureVulkan::FillImageBuffer(const ByteBuffer& buffer, const Ref<ImageBuffer>& image_buffer, int offset, int size)
	{
		auto display = (DisplayVulkan*) Graphics::GetDisplay();
		auto device = display->GetDevice();
		VkResult err;

		assert(buffer.Size() == size);

		void* mapped;
		err = vkMapMemory(device, image_buffer->GetMemory(), offset, size, 0, &mapped);
		Memory::Copy(mapped, buffer.Bytes(), buffer.Size());
		vkUnmapMemory(device, image_buffer->GetMemory());
	}

	void TextureVulkan::CopyBufferImageBegin(bool cubemap)
	{
		auto texture = (Texture*) this;
		auto mip_count = texture->GetMipmapCount();
		auto display = (DisplayVulkan*) Graphics::GetDisplay();

		VkImageSubresourceRange subresourceRange = { };
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = mip_count;
		subresourceRange.layerCount = cubemap ? 6 : 1;

		display->BeginImageCommandBuffer();

		display->SetImageLayout(
			m_image,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			(VkAccessFlagBits) 0,
			&subresourceRange);
	}

	void TextureVulkan::CopyBufferImage(const Ref<ImageBuffer>& image_buffer, int x, int y, int w, int h, bool cubemap, int face, int level)
	{
		auto texture = (Texture*) this;
		auto mip_count = texture->GetMipmapCount();
		auto display = (DisplayVulkan*) Graphics::GetDisplay();

		VkBufferImageCopy copy;
		Memory::Zero(&copy, sizeof(copy));
		copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy.imageSubresource.mipLevel = level;
		copy.imageSubresource.baseArrayLayer = face;
		copy.imageSubresource.layerCount = 1;
		copy.imageExtent.width = w;
		copy.imageExtent.height = h;
		copy.imageExtent.depth = 1;
		copy.imageOffset.x = x;
		copy.imageOffset.y = y;

		if (cubemap)
		{
			copy.bufferOffset = image_buffer->GetSize() / 6 * face;
		}

		vkCmdCopyBufferToImage(display->GetImageCommandBuffer(),
			image_buffer->GetBuffer(),
			m_image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&copy);
	}

	void TextureVulkan::CopyBufferImageEnd(bool cubemap)
	{
		auto texture = (Texture*) this;
		auto mip_count = texture->GetMipmapCount();
		auto display = (DisplayVulkan*) Graphics::GetDisplay();

		VkImageSubresourceRange subresourceRange = { };
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = mip_count;
		subresourceRange.layerCount = cubemap ? 6 : 1;

		display->SetImageLayout(
			m_image,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			&subresourceRange);

		display->EndImageCommandBuffer();
	}

	void TextureVulkan::GenerateMipmap(bool cubemap)
	{
		auto texture = (Texture*) this;
		int width = texture->GetWidth();
		int height = texture->GetHeight();
		bool mipmap = texture->IsMipmap();
		int mip_count = texture->GetMipmapCount();
		int layer_count = cubemap ? 6 : 1;

		if (mipmap == false)
		{
			return;
		}

		auto display = (DisplayVulkan*) Graphics::GetDisplay();

		display->BeginImageCommandBuffer();

		VkImageSubresourceRange range;
		Memory::Zero(&range, sizeof(range));
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = layer_count;

		display->SetImageLayout(
			m_image,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_ACCESS_SHADER_READ_BIT,
			&range);

		for (int i = 1; i < mip_count; i++)
		{
			VkImageBlit blit;
			Memory::Zero(&blit, sizeof(blit));
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = layer_count;
			blit.srcOffsets[1].x = Mathf::Max(1, width >> (i - 1));
			blit.srcOffsets[1].y = Mathf::Max(1, height >> (i - 1));
			blit.srcOffsets[1].z = 1;

			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = layer_count;
			blit.dstOffsets[1].x = Mathf::Max(1, width >> i);
			blit.dstOffsets[1].y = Mathf::Max(1, height >> i);
			blit.dstOffsets[1].z = 1;

			range.baseMipLevel = i;
			range.levelCount = 1;
			range.baseArrayLayer = 0;
			range.layerCount = layer_count;

			display->SetImageLayout(
				m_image,
				VK_IMAGE_ASPECT_COLOR_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				(VkAccessFlagBits) 0,
				&range);

			vkCmdBlitImage(
				display->GetImageCommandBuffer(),
				m_image,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				m_image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&blit,
				VK_FILTER_LINEAR);

			display->SetImageLayout(
				m_image,
				VK_IMAGE_ASPECT_COLOR_BIT,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				&range);
		}

		range.baseMipLevel = 0;
		range.levelCount = mip_count;
		range.baseArrayLayer = 0;
		range.layerCount = layer_count;

		display->SetImageLayout(
			m_image,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_TRANSFER_READ_BIT,
			&range);

		display->EndImageCommandBuffer();
	}

	void TextureVulkan::CreateCubemap()
	{
		auto texture = (Cubemap*) this;
		auto format = texture->GetFormat();

		if (format == TextureFormat::RGBA32)
		{
			m_format = VK_FORMAT_R8G8B8A8_UNORM;
		}
		else if (format == TextureFormat::RGB24)
		{
			m_format = VK_FORMAT_R8G8B8A8_UNORM;
		}
		else if (format == TextureFormat::R8)
		{
			m_format = VK_FORMAT_R8_UNORM;
		}
		else
		{
			assert(!"texture format not implement");
		}

        this->Create(VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			true);

		this->CreateView(VK_IMAGE_ASPECT_COLOR_BIT,
			{ VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A },
			true);

		this->CreateSampler();
	}

	void TextureVulkan::UpdateCubemapFaceBegin()
	{
		this->CopyBufferImageBegin(true);
	}

	void TextureVulkan::UpdateCubemapFace(int face, int level, const ByteBuffer& colors)
	{
		auto texture = (Cubemap*) this;
		int width = texture->GetWidth();
		int height = texture->GetHeight();
		auto format = texture->GetFormat();
		int buffer_size;
		ByteBuffer colors_local = colors;

		width = Mathf::Max(width >> level, 1);
		height = Mathf::Max(height >> level, 1);

		if (format == TextureFormat::RGBA32)
		{
			m_format = VK_FORMAT_R8G8B8A8_UNORM;
			buffer_size = width * height * 4;
		}
		else if (format == TextureFormat::RGB24)
		{
			m_format = VK_FORMAT_R8G8B8A8_UNORM;
			buffer_size = width * height * 4;

			int pixel_count = colors.Size() / 3;
			ByteBuffer temp(pixel_count * 4);
			for (int i = 0; i < pixel_count; i++)
			{
				temp[i * 4 + 0] = colors[i * 3 + 0];
				temp[i * 4 + 1] = colors[i * 3 + 1];
				temp[i * 4 + 2] = colors[i * 3 + 2];
				temp[i * 4 + 3] = 255;
			}
			colors_local = temp;
		}
		else if (format == TextureFormat::R8)
		{
			m_format = VK_FORMAT_R8_UNORM;
			buffer_size = width * height;
		}

		if (m_image_buffers.Size() <= level)
		{
			m_image_buffers.Resize(level + 1);
			m_image_buffers[level] = ImageBuffer::Create(buffer_size * 6);
		}

		this->FillImageBuffer(colors_local, m_image_buffers[level], buffer_size * face, buffer_size);
		this->CopyBufferImage(m_image_buffers[level], 0, 0, width, height, true, face, level);
	}

	void TextureVulkan::UpdateCubemapFaceEnd()
	{
		this->CopyBufferImageEnd(true);
	}
}

#endif
