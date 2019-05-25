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
#include "Engine.h"
#include "math/Mathf.h"
#include "memory/Memory.h"

namespace Viry3D
{
    Ref<Texture> Texture::m_shared_white_texture;
    Ref<Texture> Texture::m_shared_black_texture;
    Ref<Texture> Texture::m_shared_normal_texture;
    Ref<Texture> Texture::m_shared_cubemap;
    
    void Texture::Init()
    {
        
    }
    
    void Texture::Done()
    {
        m_shared_white_texture.reset();
        m_shared_black_texture.reset();
        m_shared_normal_texture.reset();
        m_shared_cubemap.reset();
    }
    
    const Ref<Texture>& Texture::GetSharedWhiteTexture()
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
                false);
        }
        
        return m_shared_white_texture;
    }
    
    const Ref<Texture>& Texture::GetSharedBlackTexture()
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
                false);
        }
        
        return m_shared_black_texture;
    }
    
    const Ref<Texture>& Texture::GetSharedNormalTexture()
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
                false);
        }
        
        return m_shared_normal_texture;
    }
    
    const Ref<Texture>& Texture::GetSharedCubemap()
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
            cubemap->UpdateCubemap(pixels, 0, { 0, 0, 0, 0, 0, 0 });
            m_shared_cubemap = cubemap;
        }
        
        return m_shared_cubemap;
    }
    
    Ref<Texture> Texture::LoadTexture2DFromFile(
        const String& path,
        FilterMode filter_mode,
        SamplerAddressMode wrap_mode,
        bool gen_mipmap)
    {
        Ref<Texture> texture;
        
        auto image = Image::LoadFromFile(path);
        if (image)
        {
            TextureFormat format;
            
            switch (image->format)
            {
                case ImageFormat::R8:
                    format = TextureFormat::R8;
                    break;
                case ImageFormat::R8G8B8A8:
                    format = TextureFormat::R8G8B8A8;
                    break;
                default:
                    format = TextureFormat::None;
                    break;
            }
            
            if (format != TextureFormat::None)
            {
                texture = Texture::CreateTexture2DFromMemory(
                    image->data,
                    image->width,
                    image->height,
                    format,
                    filter_mode,
                    wrap_mode,
                    gen_mipmap);
            }
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
        bool gen_mipmap)
    {
        Ref<Texture> texture = Texture::CreateTexture2D(
            width,
            height,
            format,
            filter_mode,
            wrap_mode,
            gen_mipmap);
        
        texture->UpdateTexture2D(pixels, 0, 0, width, height, 0);
        
        if (gen_mipmap)
        {
            texture->GenMipmaps();
        }
        
        return texture;
    }
    
    static filament::backend::TextureFormat GetTextureFormat(TextureFormat format)
    {
        switch (format)
        {
            case TextureFormat::R8G8B8A8:
                return filament::backend::TextureFormat::RGBA8;
			case TextureFormat::D16:
				return filament::backend::TextureFormat::DEPTH16;
			case TextureFormat::D24X8:
				return filament::backend::TextureFormat::DEPTH24;
			case TextureFormat::D24S8:
				return filament::backend::TextureFormat::DEPTH24_STENCIL8;
			case TextureFormat::D32:
				return filament::backend::TextureFormat::DEPTH32F;
			case TextureFormat::D32S8:
				return filament::backend::TextureFormat::DEPTH32F_STENCIL8;
            default:
				assert(false);
				break;
        }
		return filament::backend::TextureFormat::RGBA8;
    }
    
    static filament::backend::PixelDataFormat GetPixelDataFormat(TextureFormat format)
    {
        switch (format)
        {
            case TextureFormat::R8G8B8A8:
                return filament::backend::PixelDataFormat::RGBA;
            default:
				assert(false);
				break;
        }
		return filament::backend::PixelDataFormat::RGBA;
    }
    
    static filament::backend::PixelDataType GetPixelDataType(TextureFormat format)
    {
        switch (format)
        {
            case TextureFormat::R8G8B8A8:
                return filament::backend::PixelDataType::UBYTE;
            default:
				assert(false);
				break;
        }
		return filament::backend::PixelDataType::UBYTE;
    }
    
    Ref<Texture> Texture::CreateTexture2D(
        int width,
        int height,
        TextureFormat format,
        FilterMode filter_mode,
        SamplerAddressMode wrap_mode,
        bool mipmap)
    {
        Ref<Texture> texture;
        
        int mipmap_level_count = 1;
        if (mipmap)
        {
            mipmap_level_count = (int) floor(Mathf::Log2((float) Mathf::Max(width, height))) + 1;
        }
        
        auto& driver = Engine::Instance()->GetDriverApi();
        
        texture = Ref<Texture>(new Texture());
        texture->m_width = width;
        texture->m_height = height;
        texture->m_mipmap_level_count = mipmap_level_count;
        texture->m_array_size = 1;
        texture->m_cubemap = false;
        texture->m_format = format;
        texture->m_filter_mode = filter_mode;
        texture->m_wrap_mode = wrap_mode;
        texture->m_texture = driver.createTexture(
            filament::backend::SamplerType::SAMPLER_2D,
            texture->m_mipmap_level_count,
            GetTextureFormat(texture->m_format),
            1,
            texture->m_width,
            texture->m_height,
            1,
            filament::backend::TextureUsage::DEFAULT);

        texture->UpdateSampler();

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
            mipmap_level_count = (int) floor(Mathf::Log2((float) Mathf::Max(size, size))) + 1;
        }
        
        auto& driver = Engine::Instance()->GetDriverApi();
        
        texture = Ref<Texture>(new Texture());
        texture->m_width = size;
        texture->m_height = size;
        texture->m_mipmap_level_count = mipmap_level_count;
        texture->m_array_size = 1;
        texture->m_cubemap = true;
        texture->m_format = format;
        texture->m_filter_mode = filter_mode;
        texture->m_wrap_mode = wrap_mode;
        texture->m_texture = driver.createTexture(
            filament::backend::SamplerType::SAMPLER_CUBEMAP,
            texture->m_mipmap_level_count,
            GetTextureFormat(texture->m_format),
            1,
            texture->m_width,
            texture->m_height,
            1,
            filament::backend::TextureUsage::DEFAULT);
        
        texture->UpdateSampler();
        
        return texture;
    }

	Ref<Texture> Texture::CreateRenderTexture(
		int width,
		int height,
		TextureFormat format,
		FilterMode filter_mode,
		SamplerAddressMode wrap_mode)
	{
		Ref<Texture> texture;

		int mipmap_level_count = 1;

		auto& driver = Engine::Instance()->GetDriverApi();

		filament::backend::TextureUsage usage = filament::backend::TextureUsage::SAMPLEABLE;

		switch (format)
		{
		case TextureFormat::R8:
		case TextureFormat::R8G8:
		case TextureFormat::R8G8B8A8:
		case TextureFormat::R16G16B16A16F:
			usage |= filament::backend::TextureUsage::COLOR_ATTACHMENT;
			break;
		case TextureFormat::D16:
		case TextureFormat::D24X8:
		case TextureFormat::D32:
			usage |= filament::backend::TextureUsage::DEPTH_ATTACHMENT;
			break;
		case TextureFormat::D24S8:
		case TextureFormat::D32S8:
			usage |= filament::backend::TextureUsage::DEPTH_ATTACHMENT;
			usage |= filament::backend::TextureUsage::STENCIL_ATTACHMENT;
			break;
		case TextureFormat::S8:
			usage |= filament::backend::TextureUsage::STENCIL_ATTACHMENT;
			usage &= ~filament::backend::TextureUsage::SAMPLEABLE;
			break;
		default:
			assert(false);
			break;
		}

		texture = Ref<Texture>(new Texture());
		texture->m_width = width;
		texture->m_height = height;
		texture->m_mipmap_level_count = mipmap_level_count;
		texture->m_array_size = 1;
		texture->m_cubemap = false;
		texture->m_format = format;
		texture->m_filter_mode = filter_mode;
		texture->m_wrap_mode = wrap_mode;
		texture->m_texture = driver.createTexture(
			filament::backend::SamplerType::SAMPLER_2D,
			texture->m_mipmap_level_count,
			GetTextureFormat(texture->m_format),
			1,
			texture->m_width,
			texture->m_height,
			1,
			usage);

		texture->UpdateSampler();

		return texture;
	}
    
	TextureFormat Texture::SelectFormat(const Vector<TextureFormat>& formats, bool render_texture)
	{
		auto& driver = Engine::Instance()->GetDriverApi();
		for (int i = 0; i < formats.Size(); ++i)
		{
			if (render_texture)
			{
				if (driver.isRenderTargetFormatSupported(GetTextureFormat(formats[i])))
				{
					return formats[i];
				}
			}
			else
			{
				if (driver.isTextureFormatSupported(GetTextureFormat(formats[i])))
				{
					return formats[i];
				}
			}
			
		}

		return TextureFormat::None;
	}

	TextureFormat Texture::SelectDepthFormat()
	{
		return Texture::SelectFormat({ TextureFormat::D24X8, TextureFormat::D24S8, TextureFormat::D32, TextureFormat::D32S8, TextureFormat::D16 }, true);
	}

    Texture::Texture():
		m_width(0),
		m_height(0),
        m_mipmap_level_count(0),
        m_array_size(0),
        m_cubemap(false),
        m_format(TextureFormat::None),
        m_filter_mode(FilterMode::None),
        m_wrap_mode(SamplerAddressMode::None)
    {
        
    }
    
    Texture::~Texture()
    {
        auto& driver = Engine::Instance()->GetDriverApi();
        
        driver.destroyTexture(m_texture);
		m_texture.clear();
    }
    
    void Texture::UpdateTexture2D(const ByteBuffer& pixels, int x, int y, int w, int h, int level)
    {
        auto& driver = Engine::Instance()->GetDriverApi();
        
        void* buffer = Memory::Alloc<void>(pixels.Size());
        Memory::Copy(buffer, pixels.Bytes(), pixels.Size());
        auto data = filament::backend::PixelBufferDescriptor(
            buffer,
            pixels.Size(),
            GetPixelDataFormat(m_format),
            GetPixelDataType(m_format),
            FreeBufferCallback);
        driver.update2DImage(m_texture, level, x, y, w, h, std::move(data));
    }
    
    void Texture::UpdateCubemap(const ByteBuffer& pixels, int level, const Vector<int>& face_offsets)
    {
        auto& driver = Engine::Instance()->GetDriverApi();
        
        filament::backend::FaceOffsets offsets;
        for (int i = 0; i < 6; ++i)
        {
            offsets.offsets[i] = face_offsets[i];
        }
        
        void* buffer = Memory::Alloc<void>(pixels.Size());
        Memory::Copy(buffer, pixels.Bytes(), pixels.Size());
        auto data = filament::backend::PixelBufferDescriptor(
            buffer,
            pixels.Size(),
            GetPixelDataFormat(m_format),
            GetPixelDataType(m_format),
            FreeBufferCallback);
        driver.updateCubeImage(m_texture, level, std::move(data), offsets);
    }
    
    void Texture::GenMipmaps()
    {
        auto& driver = Engine::Instance()->GetDriverApi();
        
        if (driver.canGenerateMipmaps())
        {
            driver.generateMipmaps(m_texture);
        }
    }
    
    void Texture::UpdateSampler()
    {
        switch (m_filter_mode)
        {
            case FilterMode::None:
            case FilterMode::Nearest:
                if (m_mipmap_level_count > 1)
                {
                    m_sampler.filterMin = filament::backend::SamplerMinFilter::NEAREST_MIPMAP_NEAREST;
                }
                else
                {
                    m_sampler.filterMin = filament::backend::SamplerMinFilter::NEAREST;
                }
                m_sampler.filterMag = filament::backend::SamplerMagFilter::NEAREST;
                break;
            case FilterMode::Linear:
            case FilterMode::Trilinear:
                if (m_mipmap_level_count > 1)
                {
                    m_sampler.filterMin = filament::backend::SamplerMinFilter::LINEAR_MIPMAP_LINEAR;
                }
                else
                {
                    m_sampler.filterMin = filament::backend::SamplerMinFilter::LINEAR;
                }
                m_sampler.filterMag = filament::backend::SamplerMagFilter::LINEAR;
                break;
            default:
                break;
        }
        
        switch (m_wrap_mode)
        {
            case SamplerAddressMode::None:
            case SamplerAddressMode::ClampToEdge:
                m_sampler.wrapS = filament::backend::SamplerWrapMode::CLAMP_TO_EDGE;
                m_sampler.wrapT = filament::backend::SamplerWrapMode::CLAMP_TO_EDGE;
                m_sampler.wrapR = filament::backend::SamplerWrapMode::CLAMP_TO_EDGE;
                break;
            case SamplerAddressMode::Repeat:
                m_sampler.wrapS = filament::backend::SamplerWrapMode::REPEAT;
                m_sampler.wrapT = filament::backend::SamplerWrapMode::REPEAT;
                m_sampler.wrapR = filament::backend::SamplerWrapMode::REPEAT;
                break;
            case SamplerAddressMode::Mirror:
            case SamplerAddressMode::MirrorOnce:
                m_sampler.wrapS = filament::backend::SamplerWrapMode::MIRRORED_REPEAT;
                m_sampler.wrapT = filament::backend::SamplerWrapMode::MIRRORED_REPEAT;
                m_sampler.wrapR = filament::backend::SamplerWrapMode::MIRRORED_REPEAT;
                break;
        }
    }
}
