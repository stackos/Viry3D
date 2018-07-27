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

#include "Texture.h"
#include "Image.h"
#include "BufferObject.h"
#include "memory/Memory.h"
#include "io/File.h"
#include "math/Mathf.h"
#include "Debug.h"

namespace Viry3D
{
	Ref<Texture> Texture::m_shared_white_texture;
	Ref<Texture> Texture::m_shared_black_texture;
	Ref<Texture> Texture::m_shared_normal_texture;
	Ref<Texture> Texture::m_shared_cubemap;

    ByteBuffer Texture::LoadImageFromFile(const String& path, int& width, int& height, int& bpp)
    {
        ByteBuffer pixels;

        if (File::Exist(path))
        {
            if (path.EndsWith(".png"))
            {
                ByteBuffer png = File::ReadAllBytes(path);
                pixels = Image::LoadPNG(png, width, height, bpp);
            }
            else if (path.EndsWith(".jpg"))
            {
                ByteBuffer jpg = File::ReadAllBytes(path);
                pixels = Image::LoadJPEG(jpg, width, height, bpp);
            }
            else
            {
                assert(!"image file format not support");
            }

            if (bpp == 24)
            {
                int pixel_count = pixels.Size() / 3;
                ByteBuffer rgba(pixel_count * 4);
                for (int i = 0; i < pixel_count; i++)
                {
                    rgba[i * 4 + 0] = pixels[i * 3 + 0];
                    rgba[i * 4 + 1] = pixels[i * 3 + 1];
                    rgba[i * 4 + 2] = pixels[i * 3 + 2];
                    rgba[i * 4 + 3] = 255;
                }
                pixels = rgba;
                bpp = 32;
            }
        }

        return pixels;
    }

    Ref<Texture> Texture::LoadTexture2DFromFile(
        const String& path,
        VkFilter filter_mode,
        VkSamplerAddressMode wrap_mode,
        bool gen_mipmap)
    {
        Ref<Texture> texture;

        int width;
        int height;
        int bpp;
        ByteBuffer pixels = Texture::LoadImageFromFile(path, width, height, bpp);
        if (pixels.Size() > 0)
        {
            VkFormat format;

            if (bpp == 32)
            {
                format = VK_FORMAT_R8G8B8A8_UNORM;
            }
            else if (bpp == 8)
            {
                format = VK_FORMAT_R8_UNORM;
            }
            else
            {
                assert(!"texture format not support");
            }

            texture = Texture::CreateTexture2DFromMemory(pixels, width, height, format, filter_mode, wrap_mode, gen_mipmap, false);
        }

        return texture;
    }

    Ref<Texture> Texture::CreateTexture2DFromMemory(
        const ByteBuffer& pixels,
        int width,
        int height,
        VkFormat format,
        VkFilter filter_mode,
        VkSamplerAddressMode wrap_mode,
        bool gen_mipmap,
        bool dynamic)
    {
        Ref<Texture> texture;

        int mipmap_level_count = 1;
        if (gen_mipmap)
        {
            mipmap_level_count = (int) floor(Mathf::Log2((float) Mathf::Max(width, height))) + 1;
        }

        texture = Display::Instance()->CreateTexture(
            VK_IMAGE_TYPE_2D,
            VK_IMAGE_VIEW_TYPE_2D,
            width,
            height,
            format,
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT,
            {
                VK_COMPONENT_SWIZZLE_R,
                VK_COMPONENT_SWIZZLE_G,
                VK_COMPONENT_SWIZZLE_B,
                VK_COMPONENT_SWIZZLE_A
            },
            mipmap_level_count,
            false,
            1);
        Display::Instance()->CreateSampler(texture, filter_mode, wrap_mode);

        texture->m_dynamic = dynamic;

        texture->UpdateTexture2D(pixels, 0, 0, width, height);

        if (gen_mipmap)
        {
            texture->GenMipmaps();
        }

        return texture;
    }

    Ref<Texture> Texture::CreateCubemap(
        int size,
        VkFormat format,
        VkFilter filter_mode,
        VkSamplerAddressMode wrap_mode,
        bool mipmap)
    {
        Ref<Texture> texture;

        int mipmap_level_count = 1;
        if (mipmap)
        {
            mipmap_level_count = (int) floor(Mathf::Log2((float) size)) + 1;
        }

        texture = Display::Instance()->CreateTexture(
            VK_IMAGE_TYPE_2D,
            VK_IMAGE_VIEW_TYPE_CUBE,
            size,
            size,
            format,
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT,
            {
                VK_COMPONENT_SWIZZLE_R,
                VK_COMPONENT_SWIZZLE_G,
                VK_COMPONENT_SWIZZLE_B,
                VK_COMPONENT_SWIZZLE_A
            },
            mipmap_level_count,
            true,
            1);
        Display::Instance()->CreateSampler(texture, filter_mode, wrap_mode);

        return texture;
    }

    Ref<Texture> Texture::CreateRenderTexture(
        int width,
        int height,
        VkFormat format,
        bool create_sampler,
        VkFilter filter_mode,
        VkSamplerAddressMode wrap_mode)
    {
        Ref<Texture> texture;

        VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT;
        VkImageAspectFlags aspect;

        switch (format)
        {
            case VK_FORMAT_D16_UNORM:
            case VK_FORMAT_X8_D24_UNORM_PACK32:
            case VK_FORMAT_D32_SFLOAT:
                usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
                break;
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
                break;
            case VK_FORMAT_S8_UINT:
                usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                aspect = VK_IMAGE_ASPECT_STENCIL_BIT;
                break;
            default:
                usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                aspect = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        texture = Display::Instance()->CreateTexture(
            VK_IMAGE_TYPE_2D,
            VK_IMAGE_VIEW_TYPE_2D,
            width,
            height,
            format,
            usage,
            aspect,
            {
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY
            },
            1,
            false,
            1);
        if (create_sampler)
        {
            Display::Instance()->CreateSampler(texture, filter_mode, wrap_mode);
        }

		Display::Instance()->BeginImageCmd();
		Display::Instance()->SetImageLayout(
			texture->GetImage(),
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			{ aspect, 0, 1, 0, 1 },
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			(VkAccessFlagBits) 0);
		Display::Instance()->EndImageCmd();

        return texture;
    }

    Ref<Texture> Texture::CreateTexture2DArrayFromMemory(
        const Vector<ByteBuffer>& pixels,
        int width,
        int height,
        int layer_count,
        VkFormat format,
        VkFilter filter_mode,
        VkSamplerAddressMode wrap_mode,
        bool gen_mipmap,
        bool dynamic)
    {
        Ref<Texture> texture;

        int mipmap_level_count = 1;
        if (gen_mipmap)
        {
            mipmap_level_count = (int) floor(Mathf::Log2((float) Mathf::Max(width, height))) + 1;
        }

        texture = Display::Instance()->CreateTexture(
            VK_IMAGE_TYPE_2D,
            VK_IMAGE_VIEW_TYPE_2D_ARRAY,
            width,
            height,
            format,
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT,
            {
                VK_COMPONENT_SWIZZLE_R,
                VK_COMPONENT_SWIZZLE_G,
                VK_COMPONENT_SWIZZLE_B,
                VK_COMPONENT_SWIZZLE_A
            },
            mipmap_level_count,
            false,
            layer_count);
        Display::Instance()->CreateSampler(texture, filter_mode, wrap_mode);

        texture->m_dynamic = dynamic;

        assert(pixels.Size() == layer_count);

        for (int i = 0; i < layer_count; ++i)
        {
            texture->UpdateTexture2DArray(pixels[i], i, 0);
        }

        if (gen_mipmap)
        {
            texture->GenMipmaps();
        }

        return texture;
    }

	Ref<Texture> Texture::GetSharedWhiteTexture()
	{
		if (!m_shared_white_texture)
		{
			ByteBuffer pixels(4 * 9);
            for (int i = 0; i < 9; ++i)
            {
                pixels[i * 4 + 0] = 255;
                pixels[i * 4 + 1] = 255;
                pixels[i * 4 + 2] = 255;
                pixels[i * 4 + 3] = 255;
            }

			m_shared_white_texture = Texture::CreateTexture2DFromMemory(
				pixels,
				3,
				3,
				VK_FORMAT_R8G8B8A8_UNORM,
				VK_FILTER_NEAREST,
				VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
				false,
				false);
		}

		return m_shared_white_texture;
	}

	Ref<Texture> Texture::GetSharedBlackTexture()
	{
		if (!m_shared_black_texture)
		{
            ByteBuffer pixels(4 * 9);
            for (int i = 0; i < 9; ++i)
            {
                pixels[i * 4 + 0] = 0;
                pixels[i * 4 + 1] = 0;
                pixels[i * 4 + 2] = 0;
                pixels[i * 4 + 3] = 255;
            }

			m_shared_black_texture = Texture::CreateTexture2DFromMemory(
				pixels,
				3,
				3,
				VK_FORMAT_R8G8B8A8_UNORM,
				VK_FILTER_NEAREST,
				VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
				false,
				false);
		}

		return m_shared_black_texture;
	}

	Ref<Texture> Texture::GetSharedNormalTexture()
	{
		if (!m_shared_normal_texture)
		{
            ByteBuffer pixels(4 * 9);
            for (int i = 0; i < 9; ++i)
            {
                pixels[i * 4 + 0] = 127;
                pixels[i * 4 + 1] = 127;
                pixels[i * 4 + 2] = 255;
                pixels[i * 4 + 3] = 255;
            }

			m_shared_normal_texture = Texture::CreateTexture2DFromMemory(
				pixels,
				3,
				3,
				VK_FORMAT_R8G8B8A8_UNORM,
				VK_FILTER_NEAREST,
				VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
				false,
				false);
		}
		
		return m_shared_normal_texture;
	}

	Ref<Texture> Texture::GetSharedCubemap()
	{
		if (!m_shared_cubemap)
		{
			ByteBuffer pixels(4);
			pixels[0] = 255;
			pixels[1] = 255;
			pixels[2] = 255;
			pixels[3] = 255;

			Ref<Texture> cubemap = Texture::CreateCubemap(1, VK_FORMAT_R8G8B8A8_UNORM, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, false);
			for (int i = 0; i < 6; ++i)
			{
				cubemap->UpdateCubemap(pixels, (CubemapFace) i, 0);
			}
			m_shared_cubemap = cubemap;
		}
		
		return m_shared_cubemap;
	}

	void Texture::Done()
	{
		m_shared_white_texture.reset();
		m_shared_black_texture.reset();
		m_shared_normal_texture.reset();
		m_shared_cubemap.reset();
	}

    void Texture::UpdateTexture2D(const ByteBuffer& pixels, int x, int y, int w, int h)
    {
        VkDevice device = Display::Instance()->GetDevice();

        if (!m_image_buffer)
        {
            m_image_buffer = Display::Instance()->CreateBuffer(pixels.Bytes(), pixels.Size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        }
        else
        {
            Display::Instance()->UpdateBuffer(m_image_buffer, 0, pixels.Bytes(), pixels.Size());
        }

        this->CopyBufferToImageBegin();
        this->CopyBufferToImage(m_image_buffer, x, y, w, h, 0, 0);
        this->CopyBufferToImageEnd();

        if (!m_dynamic)
        {
            m_image_buffer->Destroy(device);
            m_image_buffer.reset();
        }
    }

    void Texture::UpdateCubemap(const ByteBuffer& pixels, CubemapFace face, int level)
    {
        VkDevice device = Display::Instance()->GetDevice();

        if (!m_image_buffer || m_image_buffer->size < pixels.Size())
        {
            m_image_buffer = Display::Instance()->CreateBuffer(pixels.Bytes(), pixels.Size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        }
        else
        {
            Display::Instance()->UpdateBuffer(m_image_buffer, 0, pixels.Bytes(), pixels.Size());
        }

        this->CopyBufferToImageBegin();
        this->CopyBufferToImage(m_image_buffer, 0, 0, m_width >> level, m_height >> level, (int) face, level);
        this->CopyBufferToImageEnd();

        if (!m_dynamic)
        {
            m_image_buffer->Destroy(device);
            m_image_buffer.reset();
        }
    }

    void Texture::UpdateTexture2DArray(const ByteBuffer& pixels, int layer, int level)
    {
        VkDevice device = Display::Instance()->GetDevice();

        if (!m_image_buffer || m_image_buffer->size < pixels.Size())
        {
            m_image_buffer = Display::Instance()->CreateBuffer(pixels.Bytes(), pixels.Size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        }
        else
        {
            Display::Instance()->UpdateBuffer(m_image_buffer, 0, pixels.Bytes(), pixels.Size());
        }

        this->CopyBufferToImageBegin();
        this->CopyBufferToImage(m_image_buffer, 0, 0, m_width >> level, m_height >> level, layer, level);
        this->CopyBufferToImageEnd();

        if (!m_dynamic)
        {
            m_image_buffer->Destroy(device);
            m_image_buffer.reset();
        }
    }

    void Texture::CopyTexture(
        const Ref<Texture>& src_texture,
        int src_layer, int src_level,
        int src_x, int src_y,
        int layer, int level,
        int x, int y,
        int w, int h)
    {
        Display::Instance()->BeginImageCmd();

        Display::Instance()->SetImageLayout(
            src_texture->GetImage(),
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            { VK_IMAGE_ASPECT_COLOR_BIT, (uint32_t) src_level, 1, (uint32_t) src_layer, 1 },
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            (VkAccessFlagBits) 0);

        Display::Instance()->SetImageLayout(
            m_image,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            { VK_IMAGE_ASPECT_COLOR_BIT, (uint32_t) level, 1, (uint32_t) layer, 1 },
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            (VkAccessFlagBits) 0);

        VkImageCopy copy;
        Memory::Zero(&copy, sizeof(copy));
        copy.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, (uint32_t) src_level, (uint32_t) src_layer, 1 };
        copy.srcOffset = { src_x, src_y, 0 };
        copy.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, (uint32_t) level, (uint32_t) layer, 1 };
        copy.dstOffset = { x, y, 0 };
        copy.extent = { (uint32_t) w, (uint32_t) h, 1 };

        vkCmdCopyImage(
            Display::Instance()->GetImageCmd(),
            src_texture->GetImage(),
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            m_image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &copy);

        Display::Instance()->SetImageLayout(
            src_texture->GetImage(),
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            { VK_IMAGE_ASPECT_COLOR_BIT, (uint32_t) src_level, 1, (uint32_t) src_layer, 1 },
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_ACCESS_TRANSFER_READ_BIT);

        Display::Instance()->SetImageLayout(
            m_image,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            { VK_IMAGE_ASPECT_COLOR_BIT, (uint32_t) level, 1, (uint32_t) layer, 1 },
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_ACCESS_TRANSFER_WRITE_BIT);

        Display::Instance()->EndImageCmd();
    }

    void Texture::CopyToMemory(ByteBuffer& pixels, int layer, int level)
    {
        Ref<BufferObject> copy_buffer = Display::Instance()->CreateBuffer(nullptr, m_width * m_height * 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

        Display::Instance()->BeginImageCmd();

        Display::Instance()->SetImageLayout(
            m_image,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            { VK_IMAGE_ASPECT_COLOR_BIT, (uint32_t) level, 1, (uint32_t) layer, 1 },
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            (VkAccessFlagBits) 0);

        VkBufferImageCopy copy;
        Memory::Zero(&copy, sizeof(copy));
        copy.bufferOffset = 0;
        copy.bufferRowLength = 0;
        copy.bufferImageHeight = 0;
        copy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, (uint32_t) level, (uint32_t) layer, 1 };
        copy.imageOffset = { 0, 0, 0 };
        copy.imageExtent = { (uint32_t) m_width, (uint32_t) m_height, 1 };

        vkCmdCopyImageToBuffer(
            Display::Instance()->GetImageCmd(),
            m_image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            copy_buffer->buffer,
            1,
            &copy);

        Display::Instance()->SetImageLayout(
            m_image,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            { VK_IMAGE_ASPECT_COLOR_BIT, (uint32_t) level, 1, (uint32_t) layer, 1 },
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_ACCESS_TRANSFER_READ_BIT);

        Display::Instance()->EndImageCmd();

        Display::Instance()->ReadBuffer(copy_buffer, pixels);

        copy_buffer->Destroy(Display::Instance()->GetDevice());
        copy_buffer.reset();
    }

    void Texture::CopyBufferToImageBegin()
    {
        Display::Instance()->BeginImageCmd();

        Display::Instance()->SetImageLayout(
            m_image,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, (uint32_t) m_mipmap_level_count, 0, (uint32_t) this->GetLayerCount() },
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            (VkAccessFlagBits) 0);
    }
    
    void Texture::CopyBufferToImage(const Ref<BufferObject>& image_buffer, int x, int y, int w, int h, int layer, int level)
    {
        VkBufferImageCopy copy;
        Memory::Zero(&copy, sizeof(copy));
        copy.bufferOffset = 0;
        copy.bufferRowLength = 0;
        copy.bufferImageHeight = 0;
        copy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, (uint32_t) level, (uint32_t) layer, 1 };
        copy.imageOffset = { x, y, 0 };
        copy.imageExtent = { (uint32_t) w, (uint32_t) h, 1 };

        vkCmdCopyBufferToImage(
            Display::Instance()->GetImageCmd(),
            image_buffer->buffer,
            m_image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &copy);
    }

    void Texture::CopyBufferToImageEnd()
    {
        Display::Instance()->SetImageLayout(
            m_image,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, (uint32_t) m_mipmap_level_count, 0, (uint32_t) this->GetLayerCount() },
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_ACCESS_TRANSFER_WRITE_BIT);

        Display::Instance()->EndImageCmd();
    }

    int Texture::GetLayerCount()
    {
        int layer_count = 1;
        if (m_cubemap)
        {
            layer_count = 6;
        }
        if (m_array_size > 1)
        {
            layer_count = m_array_size;
        }
        return layer_count;
    }

    void Texture::GenMipmaps()
    {
        assert(m_mipmap_level_count > 1);

        uint32_t layer_count = (uint32_t) this->GetLayerCount();

        Display::Instance()->BeginImageCmd();

        Display::Instance()->SetImageLayout(
            m_image,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, layer_count },
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_ACCESS_SHADER_READ_BIT);

        for (int i = 1; i < m_mipmap_level_count; ++i)
        {
            VkImageBlit blit;
            Memory::Zero(&blit, sizeof(blit));
            blit.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, (uint32_t) (i - 1), 0, layer_count };
            blit.srcOffsets[1].x = Mathf::Max(1, m_width >> (i - 1));
            blit.srcOffsets[1].y = Mathf::Max(1, m_height >> (i - 1));
            blit.srcOffsets[1].z = 1;

            blit.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, (uint32_t) i, 0, layer_count };
            blit.dstOffsets[1].x = Mathf::Max(1, m_width >> i);
            blit.dstOffsets[1].y = Mathf::Max(1, m_height >> i);
            blit.dstOffsets[1].z = 1;

            Display::Instance()->SetImageLayout(
                m_image,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
                { VK_IMAGE_ASPECT_COLOR_BIT, (uint32_t) i, 1, 0, layer_count },
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                (VkAccessFlagBits) 0);

            vkCmdBlitImage(
                Display::Instance()->GetImageCmd(),
                m_image,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                m_image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &blit,
                VK_FILTER_LINEAR);

            Display::Instance()->SetImageLayout(
                m_image,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
                { VK_IMAGE_ASPECT_COLOR_BIT, (uint32_t) i, 1, 0, layer_count },
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_ACCESS_TRANSFER_WRITE_BIT);
        }

        Display::Instance()->SetImageLayout(
            m_image,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, layer_count },
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_ACCESS_TRANSFER_READ_BIT);

        Display::Instance()->EndImageCmd();
    }

    Texture::Texture():
        m_width(0),
        m_height(0),
        m_format(VK_FORMAT_UNDEFINED),
        m_image(VK_NULL_HANDLE),
        m_image_view(VK_NULL_HANDLE),
        m_memory(VK_NULL_HANDLE),
        m_sampler(VK_NULL_HANDLE),
        m_mipmap_level_count(1),
        m_dynamic(false),
        m_cubemap(false)
    {
        Memory::Zero(&m_memory_info, sizeof(m_memory_info));
    }

    Texture::~Texture()
    {
        VkDevice device = Display::Instance()->GetDevice();

        if (m_image_buffer)
        {
            m_image_buffer->Destroy(device);
            m_image_buffer.reset();
        }
        if (m_sampler)
        {
            vkDestroySampler(device, m_sampler, nullptr);
        }
        vkDestroyImage(device, m_image, nullptr);
        vkDestroyImageView(device, m_image_view, nullptr);
        vkFreeMemory(device, m_memory, nullptr);
    }
}
