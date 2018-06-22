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

#include "Display.h"

namespace Viry3D
{
    class Texture
    {
    private:
        friend class DisplayPrivate;

    public:
        static Ref<Texture> LoadFromFile(const String& path, VkFilter filter_mode, VkSamplerAddressMode wrap_mode, bool gen_mipmap);
        static Ref<Texture> CreateFromMemory(const ByteBuffer& pixels, int width, int height, int bpp, VkFilter filter_mode, VkSamplerAddressMode wrap_mode, bool gen_mipmap);
        ~Texture();
        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }
        VkFormat GetFormat() const { return m_format; }
        VkImage GetImage() const { return m_image; }
        VkImageView GetImageView() const { return m_image_view; }

    private:
        Texture();

    private:
        int m_width;
        int m_height;
        VkFormat m_format;
        VkImage m_image;
        VkImageView m_image_view;
        VkDeviceMemory m_memory;
        VkMemoryAllocateInfo m_memory_info;
    };
}
