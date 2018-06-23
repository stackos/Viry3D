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

#include "Texture.h"
#include "Image.h"
#include "BufferObject.h"
#include "memory/Memory.h"
#include "io/File.h"
#include "math/Mathf.h"
#include <assert.h>

namespace Viry3D
{
    Ref<Texture> Texture::LoadTexture2DFromFile(
        const String& path,
        VkFilter filter_mode,
        VkSamplerAddressMode wrap_mode,
        bool gen_mipmap)
    {
        Ref<Texture> texture;

        if (path.EndsWith(".png"))
        {
            ByteBuffer png = File::ReadAllBytes(path);
            
            int width;
            int height;
            int bpp;
            ByteBuffer pixels = Image::LoadPNG(png, width, height, bpp);
            texture = Texture::CreateTexture2DFromMemory(pixels, width, height, bpp, filter_mode, wrap_mode, gen_mipmap, false);
        }
        else if (path.EndsWith(".jpg"))
        {
            ByteBuffer jpg = File::ReadAllBytes(path);

            int width;
            int height;
            int bpp;
            ByteBuffer pixels = Image::LoadJPEG(jpg, width, height, bpp);
            texture = Texture::CreateTexture2DFromMemory(pixels, width, height, bpp, filter_mode, wrap_mode, gen_mipmap, false);
        }
        else
        {
            assert(!"image file format not support");
        }

        return texture;
    }

    Ref<Texture> Texture::CreateTexture2DFromMemory(
        const ByteBuffer& pixels,
        int width,
        int height,
        int bpp,
        VkFilter filter_mode,
        VkSamplerAddressMode wrap_mode,
        bool gen_mipmap,
        bool dynamic)
    {
        Ref<Texture> texture;

        VkFormat format;
        ByteBuffer pixel_buffer = pixels;

        if (bpp == 32)
        {
            format = VK_FORMAT_R8G8B8A8_UNORM;
        }
        else if (bpp == 24)
        {
            format = VK_FORMAT_R8G8B8A8_UNORM;

            int pixel_count = pixel_buffer.Size() / 3;
            ByteBuffer rgba(pixel_count * 4);
            for (int i = 0; i < pixel_count; i++)
            {
                rgba[i * 4 + 0] = pixel_buffer[i * 3 + 0];
                rgba[i * 4 + 1] = pixel_buffer[i * 3 + 1];
                rgba[i * 4 + 2] = pixel_buffer[i * 3 + 2];
                rgba[i * 4 + 3] = 255;
            }
            pixel_buffer = rgba;
        }
        else if (bpp == 8)
        {
            format = VK_FORMAT_R8_UNORM;
        }
        else
        {
            assert(!"texture format not support");
        }

        int mipmap_level_count = 1;
        if (gen_mipmap)
        {
            mipmap_level_count = (int) floor(Mathf::Log2((float) Mathf::Max(width, height))) + 1;
        }

        texture = Display::GetDisplay()->CreateTexture(
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
            false);
        Display::GetDisplay()->CreateSampler(texture, filter_mode, wrap_mode);

        texture->m_dynamic = dynamic;

        texture->UpdateTexture2D(pixel_buffer, 0, 0, width, height);

        if (gen_mipmap)
        {
            texture->GenMipmaps();
        }

        return texture;
    }

    void Texture::UpdateTexture2D(const ByteBuffer& pixels, int x, int y, int w, int h)
    {
        VkDevice device = Display::GetDisplay()->GetDevice();

        if (!m_image_buffer)
        {
            m_image_buffer = Display::GetDisplay()->CreateBuffer(pixels.Bytes(), pixels.Size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        }
        else
        {
            Display::GetDisplay()->UpdateBuffer(m_image_buffer, 0, pixels.Bytes(), pixels.Size());
        }

        this->CopyBufferToImageBegin();
        Display::GetDisplay()->CopyBufferToImage(m_image_buffer, m_image, x, y, w, h, 0, 0);
        this->CopyBufferToImageEnd();

        if (!m_dynamic)
        {
            m_image_buffer->Destroy(device);
            m_image_buffer.reset();
        }
    }

    void Texture::CopyBufferToImageBegin()
    {
        Display::GetDisplay()->BeginImageCmd();

        Display::GetDisplay()->SetImageLayout(
            m_image,
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, (uint32_t) m_mipmap_level_count, 0, (uint32_t) (m_cubemap ? 6 : 1) },
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            (VkAccessFlagBits) 0,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT);
    }
    
    void Texture::CopyBufferToImageEnd()
    {
        Display::GetDisplay()->SetImageLayout(
            m_image,
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, (uint32_t) m_mipmap_level_count, 0, (uint32_t) (m_cubemap ? 6 : 1) },
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        Display::GetDisplay()->EndImageCmd();
    }

    void Texture::GenMipmaps()
    {
        
    }

    Texture::Texture():
        m_width(0),
        m_height(0),
        m_format(VK_FORMAT_UNDEFINED),
        m_image(nullptr),
        m_image_view(nullptr),
        m_memory(nullptr),
        m_sampler(nullptr),
        m_mipmap_level_count(1),
        m_dynamic(false),
        m_cubemap(false)
    {
        Memory::Zero(&m_memory_info, sizeof(m_memory_info));
    }

    Texture::~Texture()
    {
        VkDevice device = Display::GetDisplay()->GetDevice();

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
