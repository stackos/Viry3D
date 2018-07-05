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
#include "thread/ThreadPool.h"

namespace Viry3D
{
    enum class CubemapFace
    {
        Unknown = -1,

        PositiveX,
        NegativeX,
        PositiveY,
        NegativeY,
        PositiveZ,
        NegativeZ,

        Count
    };

    class Texture : public Thread::Res
    {
    private:
        friend class DisplayPrivate;

    public:
        static ByteBuffer LoadImageFromFile(const String& path, int& width, int& height, int& bpp);
        static Ref<Texture> LoadTexture2DFromFile(
            const String& path,
            VkFilter filter_mode,
            VkSamplerAddressMode wrap_mode,
            bool gen_mipmap);
        static Ref<Texture> CreateTexture2DFromMemory(
            const ByteBuffer& pixels,
            int width,
            int height,
            VkFormat format,
            VkFilter filter_mode,
            VkSamplerAddressMode wrap_mode,
            bool gen_mipmap,
            bool dynamic);
        static Ref<Texture> CreateCubemap(
            int size,
            VkFormat format,
            VkFilter filter_mode,
            VkSamplerAddressMode wrap_mode,
            bool mipmap);
        static Ref<Texture> CreateRenderTexture(
            int width,
            int height,
            VkFormat format,
            VkFilter filter_mode,
            VkSamplerAddressMode wrap_mode);
		static Ref<Texture> GetSharedWhiteTexture();
		static Ref<Texture> GetSharedBlackTexture();
		static Ref<Texture> GetSharedNormalTexture();
		static Ref<Texture> GetSharedCubemap();
		static void ClearSharedTextures();
        virtual ~Texture();
        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }
        int GetMipmapLevelCount() const { return m_mipmap_level_count; }
        VkFormat GetFormat() const { return m_format; }
        VkImage GetImage() const { return m_image; }
        VkImageView GetImageView() const { return m_image_view; }
        VkSampler GetSampler() const { return m_sampler; }
        void UpdateTexture2D(const ByteBuffer& pixels, int x, int y, int w, int h);
        void UpdateCubemapFaceBegin();
        void UpdateCubemapFace(const ByteBuffer& pixels, CubemapFace face, int level);
        void UpdateCubemapFaceEnd();
        void GenMipmaps();

    private:
        Texture();
        void CopyBufferToImageBegin();
        void CopyBufferToImage(const Ref<BufferObject>& image_buffer, int x, int y, int w, int h, int face, int level);
        void CopyBufferToImageEnd();

    private:
		static Ref<Texture> m_shared_white_texture;
		static Ref<Texture> m_shared_black_texture;
		static Ref<Texture> m_shared_normal_texture;
		static Ref<Texture> m_shared_cubemap;
        int m_width;
        int m_height;
        VkFormat m_format;
        VkImage m_image;
        VkImageView m_image_view;
        VkDeviceMemory m_memory;
        VkMemoryAllocateInfo m_memory_info;
        VkSampler m_sampler;
        Ref<BufferObject> m_image_buffer;
        int m_mipmap_level_count;
        bool m_dynamic;
        bool m_cubemap;
    };
}
