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

#pragma once

#include "vulkan_include.h"
#include "Object.h"

namespace Viry3D
{
	class ImageBuffer;

	class TextureVulkan: public Object
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
		void SetExternalTexture2D(void* texture) { }
		void GenerateMipmap(bool cubemap = false);
		void CreateCubemap();
		void UpdateCubemapFaceBegin();
		void UpdateCubemapFace(int face, int level, const ByteBuffer& colors);
		void UpdateCubemapFaceEnd();

	private:
		void Create(VkImageTiling tiling,
			VkImageUsageFlags usage,
			VkFlags required_props,
			VkImageLayout init_layout,
			bool cubemap);
		void CreateView(VkImageAspectFlags aspect_mask, VkComponentMapping components, bool cubemap);
		void FillImageBuffer(const ByteBuffer& buffer, const Ref<ImageBuffer>& image_buffer, int offset, int size);
		void CopyBufferImageBegin(bool cubemap = false);
		void CopyBufferImage(const Ref<ImageBuffer>& image_buffer, int x, int y, int w, int h, bool cubemap = false, int face = 0, int level = 0);
		void CopyBufferImageEnd(bool cubemap = false);
		void CreateSampler();

		VkFormat m_format;
		VkImage m_image;
		VkMemoryAllocateInfo m_memory_info;
		VkDeviceMemory m_memory;
		VkImageView m_image_view;
		VkSampler m_sampler;
		Vector<Ref<ImageBuffer>> m_image_buffers;
	};
}
