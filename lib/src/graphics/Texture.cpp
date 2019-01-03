/*
* Viry3D
* Copyright 2014-2019 by Stack - stackos@qq.com
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
#include "io/MemoryStream.h"
#include "math/Mathf.h"
#include "Debug.h"

namespace Viry3D
{
	Ref<Texture> Texture::m_shared_white_texture;
	Ref<Texture> Texture::m_shared_black_texture;
	Ref<Texture> Texture::m_shared_normal_texture;
	Ref<Texture> Texture::m_shared_cubemap;

#if VR_VULKAN
    static VkFormat TextureFormatToVkFormat(TextureFormat format)
    {
        switch (format)
        {
            case TextureFormat::R8:
                return VK_FORMAT_R8_UNORM;
            case TextureFormat::R8G8:
                return VK_FORMAT_R8G8_UNORM;
            case TextureFormat::R8G8B8A8:
                return VK_FORMAT_R8G8B8A8_UNORM;
            case TextureFormat::R16G16B16A16F:
                return VK_FORMAT_R16G16B16A16_SFLOAT;
            case TextureFormat::D16:
                return VK_FORMAT_D16_UNORM;
            case TextureFormat::D24X8:
                return VK_FORMAT_X8_D24_UNORM_PACK32;
            case TextureFormat::D32:
                return VK_FORMAT_D32_SFLOAT;
            case TextureFormat::D16S8:
                return VK_FORMAT_D16_UNORM_S8_UINT;
            case TextureFormat::D24S8:
                return VK_FORMAT_D24_UNORM_S8_UINT;
            case TextureFormat::D32S8:
                return VK_FORMAT_D32_SFLOAT_S8_UINT;
            case TextureFormat::S8:
                return VK_FORMAT_S8_UINT;
            case TextureFormat::BC1_RGB:
                return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
            case TextureFormat::BC1_RGBA:
                return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
            case TextureFormat::BC2:
                return VK_FORMAT_BC2_UNORM_BLOCK;
            case TextureFormat::BC3:
                return VK_FORMAT_BC3_UNORM_BLOCK;
            case TextureFormat::ETC2_R8G8B8:
                return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
            case TextureFormat::ETC2_R8G8B8A1:
                return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
            case TextureFormat::ETC2_R8G8B8A8:
                return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
            case TextureFormat::ASTC_4x4:
                return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
            default:
                return VK_FORMAT_UNDEFINED;
        }
    }

    static TextureFormat VkFormatToTextureFormat(VkFormat format)
    {
        switch (format)
        {
            case VK_FORMAT_R8_UNORM:
                return TextureFormat::R8;
            case VK_FORMAT_R8G8_UNORM:
                return TextureFormat::R8G8;
            case VK_FORMAT_R8G8B8A8_UNORM:
                return TextureFormat::R8G8B8A8;
            case VK_FORMAT_R16G16B16A16_SFLOAT:
                return TextureFormat::R16G16B16A16F;
            case VK_FORMAT_D16_UNORM:
                return TextureFormat::D16;
            case VK_FORMAT_X8_D24_UNORM_PACK32:
                return TextureFormat::D24X8;
            case VK_FORMAT_D32_SFLOAT:
                return TextureFormat::D32;
            case VK_FORMAT_D16_UNORM_S8_UINT:
                return TextureFormat::D16S8;
            case VK_FORMAT_D24_UNORM_S8_UINT:
                return TextureFormat::D24S8;
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                return TextureFormat::D32S8;
            case VK_FORMAT_S8_UINT:
                return TextureFormat::S8; 
            default:
                return TextureFormat::None;
        }
    }

    static VkFilter FilterModeToVkFilter(FilterMode mode)
    {
        switch (mode)
        {
            case FilterMode::None:
                return VK_FILTER_MAX_ENUM;
            case FilterMode::Nearest:
                return VK_FILTER_NEAREST;
            case FilterMode::Linear:
                return VK_FILTER_LINEAR;
            default:
                return VK_FILTER_LINEAR;
        }
    }

    static VkSamplerAddressMode SamplerAddressModeToVkMode(SamplerAddressMode mode)
    {
        switch (mode)
        {
            case SamplerAddressMode::None:
                return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
            case SamplerAddressMode::ClampToEdge:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            case SamplerAddressMode::Repeat:
                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            default:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        }
    }
#endif

    TextureFormat Texture::ChooseDepthFormatSupported(bool sample)
    {
#if VR_VULKAN
        VkFormat format = Display::Instance()->ChooseFormatSupported(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_X8_D24_UNORM_PACK32, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | (sample ? VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT : 0));
        return VkFormatToTextureFormat(format);
#elif VR_GLES
        return TextureFormat::D32;
#endif
    }

#define COMPRESSED_RGB_S3TC_DXT1 0x83F0
#define COMPRESSED_RGBA_S3TC_DXT1 0x83F1
#define COMPRESSED_RGBA_S3TC_DXT3 0x83F2
#define COMPRESSED_RGBA_S3TC_DXT5 0x83F3

#define COMPRESSED_RGB8_ETC2 0x9274
#define COMPRESSED_SRGB8_ETC2 0x9275
#define COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9276
#define COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9277
#define COMPRESSED_RGBA8_ETC2_EAC 0x9278
#define COMPRESSED_SRGB8_ALPHA8_ETC2_EAC 0x9279

#define COMPRESSED_RGBA_ASTC_4x4 0x93B0
#define COMPRESSED_RGBA_ASTC_5x4 0x93B1
#define COMPRESSED_RGBA_ASTC_5x5 0x93B2
#define COMPRESSED_RGBA_ASTC_6x5 0x93B3
#define COMPRESSED_RGBA_ASTC_6x6 0x93B4
#define COMPRESSED_RGBA_ASTC_8x5 0x93B5
#define COMPRESSED_RGBA_ASTC_8x6 0x93B6
#define COMPRESSED_RGBA_ASTC_8x8 0x93B7
#define COMPRESSED_RGBA_ASTC_10x5 0x93B8
#define COMPRESSED_RGBA_ASTC_10x6 0x93B9
#define COMPRESSED_RGBA_ASTC_10x8 0x93BA
#define COMPRESSED_RGBA_ASTC_10x10 0x93BB
#define COMPRESSED_RGBA_ASTC_12x10 0x93BC
#define COMPRESSED_RGBA_ASTC_12x12 0x93BD
#define COMPRESSED_SRGB8_ALPHA8_ASTC_4x4 0x93D0
#define COMPRESSED_SRGB8_ALPHA8_ASTC_5x4 0x93D1
#define COMPRESSED_SRGB8_ALPHA8_ASTC_5x5 0x93D2
#define COMPRESSED_SRGB8_ALPHA8_ASTC_6x5 0x93D3
#define COMPRESSED_SRGB8_ALPHA8_ASTC_6x6 0x93D4
#define COMPRESSED_SRGB8_ALPHA8_ASTC_8x5 0x93D5
#define COMPRESSED_SRGB8_ALPHA8_ASTC_8x6 0x93D6
#define COMPRESSED_SRGB8_ALPHA8_ASTC_8x8 0x93D7
#define COMPRESSED_SRGB8_ALPHA8_ASTC_10x5 0x93D8
#define COMPRESSED_SRGB8_ALPHA8_ASTC_10x6 0x93D9
#define COMPRESSED_SRGB8_ALPHA8_ASTC_10x8 0x93DA
#define COMPRESSED_SRGB8_ALPHA8_ASTC_10x10 0x93DB
#define COMPRESSED_SRGB8_ALPHA8_ASTC_12x10 0x93DC
#define COMPRESSED_SRGB8_ALPHA8_ASTC_12x12 0x93DD

    struct KTXHeader
    {
        byte identifier[12];
        uint32_t endianness;
        uint32_t type;
        uint32_t type_size;
        uint32_t format;
        uint32_t internal_format;
        uint32_t base_internal_format;
        uint32_t pixel_width;
        uint32_t pixel_height;
        uint32_t pixel_depth;
        uint32_t array_size;
        uint32_t face_count;
        uint32_t level_count;
        uint32_t key_value_data_size;
    };

    Ref<Texture> Texture::LoadFromKTXFile(
        const String& path,
        FilterMode filter_mode,
        SamplerAddressMode wrap_mode,
        bool is_storage)
    {
        Ref<Texture> texture;

        if (File::Exist(path))
        {
            MemoryStream ms(File::ReadAllBytes(path));

            KTXHeader header;

            const int identifier_size = 12;
            byte IDENTIFIER[identifier_size] = {
                0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A
            };
            ms.Read(header.identifier, identifier_size);
            if (Memory::Compare(header.identifier, IDENTIFIER, identifier_size) != 0)
            {
                return texture;
            }

            bool endian_convert = false;
            header.endianness = ms.Read<uint32_t>();
            const uint32_t ENDIAN = 0x04030201;
            if (header.endianness != ENDIAN)
            {
                endian_convert = true;

                if (header.endianness != 0x01020304)
                {
                    return texture;
                }
            }

#define READ_ENDIAN(v, t) \
            { v = ms.Read<t>(); if (endian_convert) { int left = 0; int right = sizeof(t) - 1; while (left < right) { byte* p = (byte*) &v; std::swap(p[left++], p[right--]); } } }

            READ_ENDIAN(header.type, uint32_t);
            READ_ENDIAN(header.type_size, uint32_t);
            READ_ENDIAN(header.format, uint32_t);
            if (header.type != 0 || header.type_size != 1 || header.format != 0)
            {
                Log("support compressed ktx only");
                return texture;
            }

            READ_ENDIAN(header.internal_format, uint32_t);
            READ_ENDIAN(header.base_internal_format, uint32_t);
            READ_ENDIAN(header.pixel_width, uint32_t);
            READ_ENDIAN(header.pixel_height, uint32_t);
            READ_ENDIAN(header.pixel_depth, uint32_t);
            READ_ENDIAN(header.array_size, uint32_t);
            READ_ENDIAN(header.face_count, uint32_t);
            READ_ENDIAN(header.level_count, uint32_t);
            READ_ENDIAN(header.key_value_data_size, uint32_t);

            TextureFormat texture_format = TextureFormat::None;
            int block_size_x = 4;
            int block_size_y = 4;
            int block_bit_size = 0;
            switch (header.internal_format)
            {
                case COMPRESSED_RGB_S3TC_DXT1:
                    texture_format = TextureFormat::BC1_RGB;
                    block_bit_size = 64;
                    break;
                case COMPRESSED_RGBA_S3TC_DXT1:
                    texture_format = TextureFormat::BC1_RGBA;
                    block_bit_size = 64;
                    break;
                case COMPRESSED_RGBA_S3TC_DXT3:
                    texture_format = TextureFormat::BC2;
                    block_bit_size = 128;
                    break;
                case COMPRESSED_RGBA_S3TC_DXT5:
                    texture_format = TextureFormat::BC3;
                    block_bit_size = 128;
                    break;
                case COMPRESSED_RGB8_ETC2:
                    texture_format = TextureFormat::ETC2_R8G8B8;
                    block_bit_size = 64;
                    break;
                case COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
                    texture_format = TextureFormat::ETC2_R8G8B8A1;
                    block_bit_size = 64;
                    break;
                case COMPRESSED_RGBA8_ETC2_EAC:
                    texture_format = TextureFormat::ETC2_R8G8B8A8;
                    block_bit_size = 128;
                    break;
                case COMPRESSED_RGBA_ASTC_4x4:
                    texture_format = TextureFormat::ASTC_4x4;
                    block_bit_size = 128;
                    break;
                default:
                    Log("compress format not support");
                    return texture;
            }

            uint32_t kv_size_read = 0;
            while (kv_size_read < header.key_value_data_size)
            {
                uint32_t byte_size = 0;
                READ_ENDIAN(byte_size, uint32_t);
                ByteBuffer buffer(byte_size);
                ms.Read(buffer.Bytes(), buffer.Size());
                int padding = 3 - ((byte_size + 3) % 4);
                if (padding > 0)
                {
                    ms.Read(nullptr, padding);
                }

                kv_size_read += sizeof(byte_size) + byte_size + padding;
            }

            if (header.pixel_depth > 1)
            {
                Log("3d texture not support");
                return texture;
            }

            if (header.level_count == 0)
            {
                header.level_count = 1;
            }
            if (header.array_size == 0)
            {
                header.array_size = 1;
            }
            if (header.pixel_depth == 0)
            {
                header.pixel_depth = 1;
            }
            if (header.pixel_height == 0)
            {
                header.pixel_height = 1;
            }

            bool texture_2d = false;
            bool cubemap = false;

            if (header.array_size == 1)
            {
                if (header.face_count == 1)
                {
                    texture_2d = true;
                }
                else if (header.face_count == 6)
                {
                    cubemap = true;
                }
                else
                {
                    Log("invalid face count: %d", header.face_count);
                    return texture;
                }
            }
            else
            {
                Log("texture array not support");
                return texture;
            }

            Vector<Vector<ByteBuffer>> levels;

            for (uint32_t i = 0; i < header.level_count; ++i)
            {
                uint32_t image_size = 0;
                READ_ENDIAN(image_size, uint32_t);

                uint32_t level_width = Mathf::Max(header.pixel_width >> i, 1u);
                uint32_t level_height = Mathf::Max(header.pixel_height >> i, 1u);
                int block_count_x = Mathf::Max(level_width / block_size_x, 1u);
                int block_count_y = Mathf::Max(level_height / block_size_y, 1u);

                Vector<ByteBuffer> level;

                for (uint32_t j = 0; j < header.array_size; ++j)
                {
                    for (uint32_t k = 0; k < header.face_count; ++k)
                    {
                        int face_buffer_size = block_bit_size * block_count_x * block_count_y * header.pixel_depth / 8;
                        ByteBuffer face(face_buffer_size);
                        ms.Read(face.Bytes(), face.Size());

                        int cube_padding = 3 - ((face_buffer_size + 3) % 4);
                        if (cube_padding > 0)
                        {
                            ms.Read(nullptr, cube_padding);
                        }

                        level.Add(face);
                    }
                }

                int mip_padding = 0;

                levels.Add(level);
            }

            if (texture_2d)
            {
                texture = Texture::CreateTexture2D(
                    header.pixel_width,
                    header.pixel_height,
                    texture_format,
                    filter_mode,
                    wrap_mode,
                    header.level_count > 1,
                    false,
                    is_storage);

                for (uint32_t i = 0; i < header.level_count; ++i)
                {
                    uint32_t level_width = Mathf::Max(header.pixel_width >> i, 1u);
                    uint32_t level_height = Mathf::Max(header.pixel_height >> i, 1u);

                    texture->UpdateTexture2D(levels[i][0], 0, 0, level_width, level_height, i);
                }
            }
            else if (cubemap)
            {
                texture = Texture::CreateCubemap(
                    header.pixel_width,
                    texture_format,
                    filter_mode,
                    wrap_mode,
                    header.level_count > 1);

                for (uint32_t i = 0; i < header.level_count; ++i)
                {
                    for (int j = 0; j < 6; ++j)
                    {
                        texture->UpdateCubemap(levels[i][j], (CubemapFace) j, i);
                    }
                }
            }
            
#undef READ_ENDIAN
        }

        return texture;
    }

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

            // vulkan not support R8G8B8, convert to R8G8B8A8 always
            if (bpp == 24)
            {
                int pixel_count = pixels.Size() / 3;
                ByteBuffer rgba(pixel_count * 4);
                for (int i = 0; i < pixel_count; ++i)
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
        else
        {
            Log("image file not exist: %s", path.CString());
        }

        return pixels;
    }

    Ref<Texture> Texture::LoadTexture2DFromFile(
        const String& path,
        FilterMode filter_mode,
        SamplerAddressMode wrap_mode,
        bool gen_mipmap,
        bool is_storage)
    {
        Ref<Texture> texture;

        int width;
        int height;
        int bpp;
        ByteBuffer pixels = Texture::LoadImageFromFile(path, width, height, bpp);
        if (pixels.Size() > 0)
        {
            TextureFormat format;
            
            if (bpp == 32)
            {
                format = TextureFormat::R8G8B8A8;
            }
            else if (bpp == 8)
            {
                format = TextureFormat::R8;
            }
            else
            {
                assert(!"texture format not support");
            }

            texture = Texture::CreateTexture2DFromMemory(pixels, width, height, format, filter_mode, wrap_mode, gen_mipmap, false, is_storage);
        }

        return texture;
    }

    Ref<Texture> Texture::CreateTexture2DFromMemory(
        const ByteBuffer& pixels,
        int width,
        int height,
        TextureFormat format,
        FilterMode filter_mode,
        SamplerAddressMode wrap_mode,
        bool gen_mipmap,
        bool dynamic,
        bool is_storage)
    {
        Ref<Texture> texture = Texture::CreateTexture2D(
            width,
            height,
            format,
            filter_mode,
            wrap_mode,
            gen_mipmap,
            dynamic,
            is_storage);

        texture->UpdateTexture2D(pixels, 0, 0, width, height, 0);

        if (gen_mipmap)
        {
            texture->GenMipmaps();
        }

        return texture;
    }

    Ref<Texture> Texture::CreateTexture2D(
        int width,
        int height,
        TextureFormat format,
        FilterMode filter_mode,
        SamplerAddressMode wrap_mode,
        bool mipmap,
        bool dynamic,
        bool is_storage)
    {
        Ref<Texture> texture;

        int mipmap_level_count = 1;
        if (mipmap)
        {
            mipmap_level_count = (int) floor(Mathf::Log2((float) Mathf::Max(width, height))) + 1;
        }

#if VR_VULKAN
        VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        if (is_storage)
        {
            usage |= VK_IMAGE_USAGE_STORAGE_BIT;
        }

        texture = Display::Instance()->CreateTexture(
            VK_IMAGE_TYPE_2D,
            VK_IMAGE_VIEW_TYPE_2D,
            width,
            height,
            TextureFormatToVkFormat(format),
            usage,
            VK_IMAGE_ASPECT_COLOR_BIT,
            {
                VK_COMPONENT_SWIZZLE_R,
                VK_COMPONENT_SWIZZLE_G,
                VK_COMPONENT_SWIZZLE_B,
                VK_COMPONENT_SWIZZLE_A
            },
            mipmap_level_count,
            false,
            1,
            1);
        Display::Instance()->CreateSampler(texture, FilterModeToVkFilter(filter_mode), SamplerAddressModeToVkMode(wrap_mode));

        texture->m_is_storage = is_storage;
#elif VR_GLES
        texture = CreateTexture(
            GL_TEXTURE_2D,
            width,
            height,
            format,
            mipmap_level_count);
        texture->CreateSampler(filter_mode, wrap_mode);
#endif

        texture->m_dynamic = dynamic;

        return texture;
    }

    Ref<Texture> Texture::CreateCubemap(
        int size,
        TextureFormat format,
        FilterMode filter_mode,
        SamplerAddressMode wrap_mode,
        bool mipmap)
    {
        Ref<Texture> texture;

        int mipmap_level_count = 1;
        if (mipmap)
        {
            mipmap_level_count = (int) floor(Mathf::Log2((float) size)) + 1;
        }

#if VR_VULKAN
        texture = Display::Instance()->CreateTexture(
            VK_IMAGE_TYPE_2D,
            VK_IMAGE_VIEW_TYPE_CUBE,
            size,
            size,
            TextureFormatToVkFormat(format),
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
            1,
            1);
        Display::Instance()->CreateSampler(texture, FilterModeToVkFilter(filter_mode), SamplerAddressModeToVkMode(wrap_mode));
#elif VR_GLES
        texture = CreateTexture(
            GL_TEXTURE_CUBE_MAP,
            size,
            size,
            format,
            mipmap_level_count);
        texture->CreateSampler(filter_mode, wrap_mode);
#endif

        return texture;
    }

    Ref<Texture> Texture::CreateRenderTexture(
        int width,
        int height,
        TextureFormat format,
        int array_size,
        int sample_count,
        bool create_sampler,
        FilterMode filter_mode,
        SamplerAddressMode wrap_mode)
    {
        Ref<Texture> texture;

#if VR_VULKAN
        VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT;
        VkImageAspectFlags aspect;

        switch (format)
        {
            case TextureFormat::D16:
            case TextureFormat::D24X8:
            case TextureFormat::D32:
                usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
                break;
            case TextureFormat::D16S8:
            case TextureFormat::D24S8:
            case TextureFormat::D32S8:
                usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
                break;
            case TextureFormat::S8:
                usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                aspect = VK_IMAGE_ASPECT_STENCIL_BIT;
                break;
            default:
                usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                aspect = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        texture = Display::Instance()->CreateTexture(
            VK_IMAGE_TYPE_2D,
            array_size > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D,
            width,
            height,
            TextureFormatToVkFormat(format),
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
            array_size,
            sample_count);
        if (create_sampler)
        {
            Display::Instance()->CreateSampler(texture, FilterModeToVkFilter(filter_mode), SamplerAddressModeToVkMode(wrap_mode));
        }

		Display::Instance()->BeginImageCmd();
		Display::Instance()->SetImageLayout(
			texture->GetImage(),
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			{ aspect, 0, 1, 0, (uint32_t) array_size },
			VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_GENERAL,
			(VkAccessFlagBits) 0);
        if (sample_count > 1)
        {
            Display::Instance()->SetImageLayout(
                texture->GetImageMultiSample(),
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                { aspect, 0, 1, 0, (uint32_t) array_size },
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_GENERAL,
                (VkAccessFlagBits) 0);
        }
		Display::Instance()->EndImageCmd();
#elif VR_GLES
        texture = CreateTexture(
            GL_TEXTURE_2D,
            width,
            height,
            format,
            1);
        if (create_sampler)
        {
            texture->CreateSampler(filter_mode, wrap_mode);
        }

        texture->UpdateTexture2D(ByteBuffer(), 0, 0, width, height, 0);

        texture->m_render_texture = true;

        texture->m_sample_count = sample_count;
        if (sample_count > 1)
        {
            texture->CreateRenderbufferMultiSample();
        }
#endif

        return texture;
    }

    Ref<Texture> Texture::CreateTexture2DArrayFromMemory(
        const Vector<ByteBuffer>& pixels,
        int width,
        int height,
        int layer_count,
        TextureFormat format,
        FilterMode filter_mode,
        SamplerAddressMode wrap_mode,
        bool gen_mipmap,
        bool dynamic)
    {
        Ref<Texture> texture;

        int mipmap_level_count = 1;
        if (gen_mipmap)
        {
            mipmap_level_count = (int) floor(Mathf::Log2((float) Mathf::Max(width, height))) + 1;
        }

#if VR_VULKAN
        texture = Display::Instance()->CreateTexture(
            VK_IMAGE_TYPE_2D,
            VK_IMAGE_VIEW_TYPE_2D_ARRAY,
            width,
            height,
            TextureFormatToVkFormat(format),
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
            layer_count,
            1);
        Display::Instance()->CreateSampler(texture, FilterModeToVkFilter(filter_mode), SamplerAddressModeToVkMode(wrap_mode));
#endif

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

    Ref<Texture> Texture::CreateStorageTexture2D(
        int width,
        int height,
        TextureFormat format,
        bool create_sampler,
        FilterMode filter_mode,
        SamplerAddressMode wrap_mode)
    {
        Ref<Texture> texture;

#if VR_VULKAN
        texture = Display::Instance()->CreateTexture(
            VK_IMAGE_TYPE_2D,
            VK_IMAGE_VIEW_TYPE_2D,
            width,
            height,
            TextureFormatToVkFormat(format),
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT,
            {
                VK_COMPONENT_SWIZZLE_R,
                VK_COMPONENT_SWIZZLE_G,
                VK_COMPONENT_SWIZZLE_B,
                VK_COMPONENT_SWIZZLE_A
            },
            1,
            false,
            1,
            1);
        if (create_sampler)
        {
            Display::Instance()->CreateSampler(texture, FilterModeToVkFilter(filter_mode), SamplerAddressModeToVkMode(wrap_mode));
        }

        texture->m_is_storage = true;

        Display::Instance()->BeginImageCmd();
        Display::Instance()->SetImageLayout(
            texture->GetImage(),
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_GENERAL,
            (VkAccessFlagBits) 0);
        Display::Instance()->EndImageCmd();
#endif

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
                TextureFormat::R8G8B8A8,
                FilterMode::Nearest,
                SamplerAddressMode::ClampToEdge,
				false,
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
                TextureFormat::R8G8B8A8,
                FilterMode::Nearest,
                SamplerAddressMode::ClampToEdge,
				false,
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
                TextureFormat::R8G8B8A8,
                FilterMode::Nearest,
                SamplerAddressMode::ClampToEdge,
				false,
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

			Ref<Texture> cubemap = Texture::CreateCubemap(
                1,
                TextureFormat::R8G8B8A8,
                FilterMode::Nearest,
                SamplerAddressMode::ClampToEdge,
                false);
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

    void Texture::UpdateTexture2D(const ByteBuffer& pixels, int x, int y, int w, int h, int level)
    {
#if VR_VULKAN
        VkDevice device = Display::Instance()->GetDevice();

        if (!m_image_buffer)
        {
            m_image_buffer = Display::Instance()->CreateBuffer(pixels.Bytes(), pixels.Size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, false, VK_FORMAT_UNDEFINED);
        }
        else
        {
            Display::Instance()->UpdateBuffer(m_image_buffer, 0, pixels.Bytes(), pixels.Size());
        }

        this->CopyBufferToImageBegin();
        this->CopyBufferToImage(m_image_buffer, x, y, w, h, 0, level);
        this->CopyBufferToImageEnd();

        if (!m_dynamic)
        {
            m_image_buffer->Destroy(device);
            m_image_buffer.reset();
        }
#elif VR_GLES
        this->Bind();

        if (level < m_have_storage.Size() && m_have_storage[level])
        {
            glTexSubImage2D(m_target, level, x, y, w, h, m_format, m_pixel_type, pixels.Bytes());
        }
        else
        {
            if (level >= m_have_storage.Size())
            {
                m_have_storage.Resize(level + 1);
            }
            m_have_storage[level] = GL_TRUE;

            int level_w = Mathf::Max(1, m_width >> level);
            int level_h = Mathf::Max(1, m_height >> level);
            if (x == 0 && y == 0 && w == level_w && h == level_h)
            {
                glTexImage2D(m_target, level, m_internal_format, level_w, level_h, 0, m_format, m_pixel_type, pixels.Bytes());
            }
            else
            {
                glTexImage2D(m_target, level, m_internal_format, level_w, level_h, 0, m_format, m_pixel_type, nullptr);
                glTexSubImage2D(m_target, level, x, y, w, h, m_format, m_pixel_type, pixels.Bytes());
            }
        }

        this->Unbind();
#endif
    }

    void Texture::UpdateCubemap(const ByteBuffer& pixels, CubemapFace face, int level)
    {
#if VR_VULKAN
        VkDevice device = Display::Instance()->GetDevice();

        if (!m_image_buffer || m_image_buffer->GetSize() < pixels.Size())
        {
            m_image_buffer = Display::Instance()->CreateBuffer(pixels.Bytes(), pixels.Size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, false, VK_FORMAT_UNDEFINED);
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
#elif VR_GLES
        this->Bind();

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + (int) face, level, m_internal_format, m_width >> level, m_height >> level, 0, m_format, m_pixel_type, pixels.Bytes());

        this->Unbind();
#endif
    }

    void Texture::UpdateTexture2DArray(const ByteBuffer& pixels, int layer, int level)
    {
#if VR_VULKAN
        VkDevice device = Display::Instance()->GetDevice();

        if (!m_image_buffer || m_image_buffer->GetSize() < pixels.Size())
        {
            m_image_buffer = Display::Instance()->CreateBuffer(pixels.Bytes(), pixels.Size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, false, VK_FORMAT_UNDEFINED);
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
#endif
    }

#if VR_VULKAN
    void Texture::CopyTexture(
        const Ref<Texture>& src_texture,
        int src_layer, int src_level,
        int src_x, int src_y,
        int src_w, int src_h,
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

        if (src_w == w && src_h == h)
        {
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
        }
        else
        {
            VkImageBlit blit;
            Memory::Zero(&blit, sizeof(blit));
            blit.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, (uint32_t) src_level, (uint32_t) src_layer, 1 };
            blit.srcOffsets[1] = { src_w, src_h, 1 };
            blit.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, (uint32_t) level, (uint32_t) layer, 1 };
            blit.dstOffsets[1] = { w, h, 1 };

            vkCmdBlitImage(
                Display::Instance()->GetImageCmd(),
                src_texture->GetImage(),
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                m_image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &blit,
                VK_FILTER_LINEAR);
        }

        Display::Instance()->SetImageLayout(
            src_texture->GetImage(),
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            { VK_IMAGE_ASPECT_COLOR_BIT, (uint32_t) src_level, 1, (uint32_t) src_layer, 1 },
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_ACCESS_TRANSFER_READ_BIT);

        Display::Instance()->SetImageLayout(
            m_image,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            { VK_IMAGE_ASPECT_COLOR_BIT, (uint32_t) level, 1, (uint32_t) layer, 1 },
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_ACCESS_TRANSFER_WRITE_BIT);

        Display::Instance()->EndImageCmd();
    }

    void Texture::CopyToMemory(ByteBuffer& pixels, int layer, int level)
    {
        Ref<BufferObject> copy_buffer = Display::Instance()->CreateBuffer(nullptr, m_width * m_height * 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT, false, VK_FORMAT_UNDEFINED);

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
            copy_buffer->GetBuffer(),
            1,
            &copy);

        Display::Instance()->SetImageLayout(
            m_image,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            { VK_IMAGE_ASPECT_COLOR_BIT, (uint32_t) level, 1, (uint32_t) layer, 1 },
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_LAYOUT_GENERAL,
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
            image_buffer->GetBuffer(),
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
            VK_IMAGE_LAYOUT_GENERAL,
            VK_ACCESS_TRANSFER_WRITE_BIT);

        Display::Instance()->EndImageCmd();
    }
#elif VR_GLES
    void Texture::CopyTexture(
        const Ref<Texture>& src_texture,
        int src_layer, int src_level,
        int src_x, int src_y,
        int src_w, int src_h,
        int layer, int level,
        int x, int y,
        int w, int h)
    {
        this->Bind();

        if (src_w == w && src_h == h)
        {
            if (m_copy_framebuffer == 0)
            {
                glGenFramebuffers(1, &m_copy_framebuffer);
            }
            glBindFramebuffer(GL_FRAMEBUFFER, m_copy_framebuffer);
            if (src_texture->GetTarget() == GL_TEXTURE_CUBE_MAP)
            {
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + src_layer, src_texture->GetTexture(), src_level);
            }
            else
            {
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, src_texture->GetTarget(), src_texture->GetTexture(), src_level);
            }

            if (m_target == GL_TEXTURE_CUBE_MAP)
            {
                glCopyTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer, level, x, y, src_x, src_y, w, h);
            }
            else
            {
                glCopyTexSubImage2D(m_target, level, x, y, src_x, src_y, w, h);
            }
        }
        else
        {
            Log("texture scale copy not implement in gl");
        }
        
#if VR_IOS
        Display::Instance()->BindDefaultFramebuffer();
#else
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif

        this->Unbind();
    }

    void Texture::CopyToMemory(ByteBuffer& pixels, int layer, int level)
    {
        this->Bind();

        if (m_copy_framebuffer == 0)
        {
            glGenFramebuffers(1, &m_copy_framebuffer);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, m_copy_framebuffer);
        if (m_target == GL_TEXTURE_CUBE_MAP)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer, m_texture, level);
        }
        else
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_target, m_texture, level);
        }

        glReadPixels(0, 0, m_width, m_height, m_format, m_pixel_type, pixels.Bytes());

#if VR_IOS
        Display::Instance()->BindDefaultFramebuffer();
#else
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
        
        this->Unbind();
    }
#endif

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

#if VR_VULKAN
        uint32_t layer_count = (uint32_t) this->GetLayerCount();

        Display::Instance()->BeginImageCmd();

        Display::Instance()->SetImageLayout(
            m_image,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, layer_count },
            VK_IMAGE_LAYOUT_GENERAL,
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
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, (uint32_t) m_mipmap_level_count, 0, layer_count },
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_ACCESS_TRANSFER_READ_BIT);

        Display::Instance()->EndImageCmd();
#elif VR_GLES
        this->Bind();

        glGenerateMipmap(m_target);

        this->Unbind();
#endif
    }

    Texture::Texture():
#if VR_VULKAN
        m_format(VK_FORMAT_UNDEFINED),
        m_image(VK_NULL_HANDLE),
        m_image_view(VK_NULL_HANDLE),
        m_memory(VK_NULL_HANDLE),
        m_image_multi_sample(VK_NULL_HANDLE),
        m_image_view_multi_sample(VK_NULL_HANDLE),
        m_memory_multi_sample(VK_NULL_HANDLE),
        m_sampler(VK_NULL_HANDLE),
        m_is_storage(false),
#elif VR_GLES
        m_texture(0),
        m_target(0),
        m_internal_format(0),
        m_format(0),
        m_pixel_type(0),
        m_copy_framebuffer(0),
        m_render_texture(false),
        m_depth_texture(false),
        m_renderbuffer_multi_sample(0),
#endif
        m_width(0),
        m_height(0),
        m_mipmap_level_count(1),
        m_dynamic(false),
        m_cubemap(false),
        m_array_size(1),
        m_sample_count(1)
    {
#if VR_VULKAN
        Memory::Zero(&m_memory_info, sizeof(m_memory_info));
        Memory::Zero(&m_memory_info_multi_sample, sizeof(m_memory_info_multi_sample));
#endif
    }

    Texture::~Texture()
    {
#if VR_VULKAN
        VkDevice device = Display::Instance()->GetDevice();

        // All submitted commands that refer to image, either directly or via a VkImageView, must have completed execution
        Display::Instance()->WaitDevice();

        if (m_image_buffer)
        {
            m_image_buffer->Destroy(device);
            m_image_buffer.reset();
        }
        if (m_sampler)
        {
            vkDestroySampler(device, m_sampler, nullptr);
        }
        if (m_sample_count > 1)
        {
            vkDestroyImage(device, m_image_multi_sample, nullptr);
            vkDestroyImageView(device, m_image_view_multi_sample, nullptr);
            vkFreeMemory(device, m_memory_multi_sample, nullptr);
        }
        vkDestroyImage(device, m_image, nullptr);
        vkDestroyImageView(device, m_image_view, nullptr);
        vkFreeMemory(device, m_memory, nullptr);
#elif VR_GLES
        if (m_texture)
        {
            glDeleteTextures(1, &m_texture);
        }
        if (m_copy_framebuffer)
        {
            glDeleteFramebuffers(1, &m_copy_framebuffer);
        }
        if (m_renderbuffer_multi_sample)
        {
            glDeleteRenderbuffers(1, &m_renderbuffer_multi_sample);
        }
#endif
    }

#if VR_GLES
    Ref<Texture> Texture::CreateTexture(
        GLenum target,
        int width,
        int height,
        TextureFormat format,
        int mipmap_level_count)
    {
        Ref<Texture> texture = Ref<Texture>(new Texture());

        glGenTextures(1, &texture->m_texture);
        texture->m_target = target;
        texture->m_width = width;
        texture->m_height = height;
        texture->m_mipmap_level_count = mipmap_level_count;
        texture->m_cubemap = target == GL_TEXTURE_CUBE_MAP;

        switch (format)
        {
        case TextureFormat::R8:
#if VR_WINDOWS || VR_MAC
            texture->m_internal_format = GL_RED;
            texture->m_format = GL_RED;
#else
            texture->m_internal_format = GL_LUMINANCE;
            texture->m_format = GL_LUMINANCE;
#endif
            texture->m_pixel_type = GL_UNSIGNED_BYTE;
            break;
        case TextureFormat::R8G8:
#if VR_WINDOWS || VR_MAC
            texture->m_internal_format = GL_RG;
            texture->m_format = GL_RG;
            texture->m_pixel_type = GL_UNSIGNED_BYTE;
#else
            texture->m_internal_format = GL_LUMINANCE_ALPHA;
            texture->m_format = GL_LUMINANCE_ALPHA;
            texture->m_pixel_type = GL_UNSIGNED_BYTE;
#endif
            break;
        case TextureFormat::R8G8B8A8:
            if (Display::Instance()->IsGLESv3())
            {
                texture->m_internal_format = GL_RGBA8;
            }
            else
            {
                texture->m_internal_format = GL_RGBA;
            }
            texture->m_format = GL_RGBA;
            texture->m_pixel_type = GL_UNSIGNED_BYTE;
            break;
        case TextureFormat::D16:
            if (Display::Instance()->IsGLESv3())
            {
                texture->m_internal_format = GL_DEPTH_COMPONENT16;
                texture->m_format = GL_DEPTH_COMPONENT;
                texture->m_pixel_type = GL_UNSIGNED_SHORT;
            }
            else
            {
                texture->m_internal_format = GL_DEPTH_COMPONENT;
                texture->m_format = GL_DEPTH_COMPONENT;
                texture->m_pixel_type = GL_UNSIGNED_SHORT;
            }
            texture->m_depth_texture = true;
            break;
        case TextureFormat::D32:
            if (Display::Instance()->IsGLESv3())
            {
                texture->m_internal_format = GL_DEPTH_COMPONENT32F;
                texture->m_format = GL_DEPTH_COMPONENT;
                texture->m_pixel_type = GL_FLOAT;
            }
            else
            {
                texture->m_internal_format = GL_DEPTH_COMPONENT;
                texture->m_format = GL_DEPTH_COMPONENT;
                texture->m_pixel_type = GL_UNSIGNED_INT;
            }
            texture->m_depth_texture = true;
            break;
        case TextureFormat::D24X8:
            if (Display::Instance()->IsGLESv3())
            {
                texture->m_internal_format = GL_DEPTH_COMPONENT24;
                texture->m_format = GL_DEPTH_COMPONENT;
                texture->m_pixel_type = GL_UNSIGNED_INT;
            }
            else
            {
                Log("texture format not support: %d", format);
            }
            texture->m_depth_texture = true;
            break;
        case TextureFormat::D24S8:
            if (Display::Instance()->IsGLESv3())
            {
                texture->m_internal_format = GL_DEPTH24_STENCIL8;
                texture->m_format = GL_DEPTH_STENCIL;
                texture->m_pixel_type = GL_UNSIGNED_INT_24_8;
            }
            else
            {
                Log("texture format not support: %d", format);
            }
            texture->m_depth_texture = true;
            break;
        case TextureFormat::D32S8:
            if (Display::Instance()->IsGLESv3())
            {
                texture->m_internal_format = GL_DEPTH32F_STENCIL8;
                texture->m_format = GL_DEPTH_STENCIL;
                texture->m_pixel_type = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
            }
            else
            {
                Log("texture format not support: %d", format);
            }
            texture->m_depth_texture = true;
            break;
        default:
            Log("texture format not support: %d", format);
            break;
        }

        return texture;
    }

    void Texture::CreateSampler(FilterMode filter_mode, SamplerAddressMode wrap_mode)
    {
        GLint min_filter = 0;
        GLint mag_filter = 0;
        GLint wrap = 0;

        // MARK:
        // GLESv3 depth texture not support linear filter,
        // so use nearest filter always.
        if (m_depth_texture)
        {
            min_filter = GL_NEAREST;
            mag_filter = GL_NEAREST;
        }
        else
        {
            switch (filter_mode)
            {
            case FilterMode::Nearest:
                if (m_mipmap_level_count > 1)
                {
                    min_filter = GL_NEAREST_MIPMAP_NEAREST;
                }
                else
                {
                    min_filter = GL_NEAREST;
                }
                mag_filter = GL_NEAREST;
                break;

            case FilterMode::Linear:
                if (m_mipmap_level_count > 1)
                {
                    min_filter = GL_LINEAR_MIPMAP_NEAREST;
                }
                else
                {
                    min_filter = GL_LINEAR;
                }
                mag_filter = GL_LINEAR;
                break;

            case FilterMode::Trilinear:
                if (m_mipmap_level_count > 1)
                {
                    min_filter = GL_LINEAR_MIPMAP_LINEAR;
                }
                else
                {
                    min_filter = GL_LINEAR;
                }
                mag_filter = GL_LINEAR;
                break;
            }
        }

        switch (wrap_mode)
        {
            case SamplerAddressMode::Repeat:
                wrap = GL_REPEAT;
                break;
            case SamplerAddressMode::ClampToEdge:
                wrap = GL_CLAMP_TO_EDGE;
                break;
            case SamplerAddressMode::Mirror:
                wrap = GL_MIRRORED_REPEAT;
                break;
            case SamplerAddressMode::MirrorOnce:
                Log("texture wrap mode not support: MirrorOnce");
                break;
        }

        this->Bind();

        glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, min_filter);
        glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, mag_filter);
        glTexParameteri(m_target, GL_TEXTURE_WRAP_S, wrap);
        glTexParameteri(m_target, GL_TEXTURE_WRAP_T, wrap);

        this->Unbind();
    }

    void Texture::CreateRenderbufferMultiSample()
    {
        glGenRenderbuffers(1, &m_renderbuffer_multi_sample);
        glBindRenderbuffer(GL_RENDERBUFFER, m_renderbuffer_multi_sample);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_sample_count, m_internal_format, m_width, m_height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
#endif
}
