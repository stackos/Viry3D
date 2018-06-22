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
#include "memory/Memory.h"
#include "io/File.h"
#include <assert.h>

namespace Viry3D
{
    Ref<Texture> Texture::LoadFromFile(const String& path, VkFilter filter_mode, VkSamplerAddressMode wrap_mode, bool gen_mipmap)
    {
        Ref<Texture> texture;

        if (path.EndsWith(".png"))
        {
            ByteBuffer png = File::ReadAllBytes(path);
            
            int width;
            int height;
            int bpp;
            ByteBuffer pixels = Image::LoadPNG(png, width, height, bpp);
            texture = Texture::CreateFromMemory(pixels, width, height, bpp, filter_mode, wrap_mode, gen_mipmap);
        }
        else if (path.EndsWith(".jpg"))
        {
            ByteBuffer jpg = File::ReadAllBytes(path);

            int width;
            int height;
            int bpp;
            ByteBuffer pixels = Image::LoadJPEG(jpg, width, height, bpp);
            texture = Texture::CreateFromMemory(pixels, width, height, bpp, filter_mode, wrap_mode, gen_mipmap);
        }
        else
        {
            assert(!"image file format not support");
        }

        return texture;
    }

    Ref<Texture> Texture::CreateFromMemory(const ByteBuffer& pixels, int width, int height, int bpp, VkFilter filter_mode, VkSamplerAddressMode wrap_mode, bool gen_mipmap)
    {
        Ref<Texture> texture;

        return texture;
    }

    Texture::Texture():
        m_width(0),
        m_height(0),
        m_format(VK_FORMAT_UNDEFINED),
        m_image(nullptr),
        m_image_view(nullptr),
        m_memory(nullptr)
    {
        Memory::Zero(&m_memory_info, sizeof(m_memory_info));
    }

    Texture::~Texture()
    {
        VkDevice device = Display::GetDisplay()->GetDevice();

        vkDestroyImage(device, m_image, nullptr);
        vkDestroyImageView(device, m_image_view, nullptr);
        vkFreeMemory(device, m_memory, nullptr);
    }
}
