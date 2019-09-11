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
#include "io/MemoryStream.h"
#include "io/File.h"
#include "Debug.h"

namespace Viry3D
{
	Ref<Image> Texture::m_shared_white_image;
	Ref<Texture> Texture::m_shared_white_texture;
	Ref<Texture> Texture::m_shared_black_texture;
	Ref<Texture> Texture::m_shared_normal_texture;
	Ref<Texture> Texture::m_shared_cubemap;

	void Texture::Init()
	{

	}

	void Texture::Done()
	{
		m_shared_white_image.reset();
		m_shared_white_texture.reset();
		m_shared_black_texture.reset();
		m_shared_normal_texture.reset();
		m_shared_cubemap.reset();
	}

	const Ref<Image>& Texture::GetSharedWhiteImage()
	{
		if (!m_shared_white_image)
		{
			ByteBuffer pixels(4 * 9);
			for (int i = 0; i < 9; ++i)
			{
				pixels[i * 4 + 0] = 255;
				pixels[i * 4 + 1] = 255;
				pixels[i * 4 + 2] = 255;
				pixels[i * 4 + 3] = 255;
			}

			m_shared_white_image = RefMake<Image>();
			m_shared_white_image->width = 3;
			m_shared_white_image->height = 3;
			m_shared_white_image->format = ImageFormat::R8G8B8A8;
			m_shared_white_image->data = pixels;
		}

		return m_shared_white_image;
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

#define COMPRESSED_RGB8_ETC1 0x8D64
    
#define COMPRESSED_RGB8_PVRTC_4V1 0x8C00
#define COMPRESSED_RGBA8_PVRTC_4V1 0x8C02
    
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
		SamplerAddressMode wrap_mode)
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
                case COMPRESSED_RGB8_ETC1:
                    texture_format = TextureFormat::ETC1_R8G8B8;
                    block_bit_size = 64;
                    break;
                case COMPRESSED_RGB8_PVRTC_4V1:
                    texture_format = TextureFormat::PVRTC_R8G8B8_4V1;
                    block_bit_size = 64;
                    break;
                case COMPRESSED_RGBA8_PVRTC_4V1:
                    texture_format = TextureFormat::PVRTC_R8G8B8A8_4V1;
                    block_bit_size = 64;
                    break;
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
					header.level_count > 1);

				for (uint32_t i = 0; i < header.level_count; ++i)
				{
					uint32_t level_width = Mathf::Max(header.pixel_width >> i, 1u);
					uint32_t level_height = Mathf::Max(header.pixel_height >> i, 1u);

					texture->UpdateTexture(levels[i][0], 0, i, 0, 0, level_width, level_height);
				}

				texture->m_file_path = path;
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
					ByteBuffer buffer;
					Vector<int> offsets(6);

					for (int j = 0; j < 6; ++j)
					{
						const auto& face = levels[i][j];
						if (buffer.Size() == 0)
						{
							buffer = ByteBuffer(face.Size() * 6);
						}
						Memory::Copy(&buffer[j * face.Size()], face.Bytes(), face.Size());
						offsets[j] = j * face.Size();
					}

					if (buffer.Size() > 0)
					{
						texture->UpdateCubemap(buffer, i, offsets);
					}
				}

				texture->m_file_path = path;
			}

#undef READ_ENDIAN
		}

		return texture;
	}

	Ref<Texture> Texture::LoadTexture2DFromFile(
		const String& path,
		FilterMode filter_mode,
		SamplerAddressMode wrap_mode,
		bool gen_mipmap)
	{
		Ref<Texture> texture;

		if (File::Exist(path))
		{
			ByteBuffer image_buffer = File::ReadAllBytes(path);
			texture = Texture::LoadTexture2DFromMemory(
				image_buffer,
				filter_mode,
				wrap_mode,
				gen_mipmap);

			texture->m_file_path = path;
		}

		return texture;
	}

	Ref<Texture> Texture::LoadTexture2DFromMemory(
		const ByteBuffer& image_buffer,
		FilterMode filter_mode,
		SamplerAddressMode wrap_mode,
		bool gen_mipmap)
	{
		Ref<Texture> texture;

		auto image = Image::LoadFromMemory(image_buffer);
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

		texture->UpdateTexture(pixels, 0, 0, 0, 0, width, height);

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
			case TextureFormat::R8:
				return filament::backend::TextureFormat::R8;
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

            case TextureFormat::ETC1_R8G8B8:
                return filament::backend::TextureFormat::ETC1_RGB8;
            case TextureFormat::PVRTC_R8G8B8_4V1:
                return filament::backend::TextureFormat::PVRTC_RGB8_4V1;
            case TextureFormat::PVRTC_R8G8B8A8_4V1:
                return filament::backend::TextureFormat::PVRTC_RGBA8_4V1;
			case TextureFormat::BC1_RGB:
				return filament::backend::TextureFormat::DXT1_RGB;
			case TextureFormat::BC1_RGBA:
				return filament::backend::TextureFormat::DXT1_RGBA;
			case TextureFormat::BC2:
				return filament::backend::TextureFormat::DXT3_RGBA;
			case TextureFormat::BC3:
				return filament::backend::TextureFormat::DXT5_RGBA;
            case TextureFormat::ETC2_R8G8B8:
                return filament::backend::TextureFormat::ETC2_RGB8;
            case TextureFormat::ETC2_R8G8B8A1:
                return filament::backend::TextureFormat::ETC2_RGB8_A1;
            case TextureFormat::ETC2_R8G8B8A8:
                return filament::backend::TextureFormat::ETC2_EAC_RGBA8;
            case TextureFormat::ASTC_4x4:
                return filament::backend::TextureFormat::RGBA_ASTC_4x4;
			default:
				assert(false);
				break;
		}
		return filament::backend::TextureFormat::RGBA8;
	}

	static bool IsCompressedFormat(TextureFormat format)
	{
		switch (format)
		{
            case TextureFormat::ETC1_R8G8B8:
            case TextureFormat::PVRTC_R8G8B8_4V1:
            case TextureFormat::PVRTC_R8G8B8A8_4V1:
			case TextureFormat::BC1_RGB:
			case TextureFormat::BC1_RGBA:
			case TextureFormat::BC2:
			case TextureFormat::BC3:
			case TextureFormat::ETC2_R8G8B8:
			case TextureFormat::ETC2_R8G8B8A1:
			case TextureFormat::ETC2_R8G8B8A8:
			case TextureFormat::ASTC_4x4:
				return true;
			default:
				return false;
		}
	}

	static filament::backend::PixelDataFormat GetPixelDataFormat(TextureFormat format)
	{
		switch (format)
		{
            case TextureFormat::R8:
                return filament::backend::PixelDataFormat::R;
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
            case TextureFormat::R8:
			case TextureFormat::R8G8B8A8:
				return filament::backend::PixelDataType::UBYTE;
			default:
				assert(false);
				break;
		}
		return filament::backend::PixelDataType::UBYTE;
	}

	static filament::backend::CompressedPixelDataType GetCompressedPixelDataType(TextureFormat format)
	{
		switch (format)
		{
            case TextureFormat::ETC1_R8G8B8:
                return filament::backend::CompressedPixelDataType::ETC1_RGB8;
            case TextureFormat::PVRTC_R8G8B8_4V1:
                return filament::backend::CompressedPixelDataType::PVRTC_RGB8_4V1;
            case TextureFormat::PVRTC_R8G8B8A8_4V1:
                return filament::backend::CompressedPixelDataType::PVRTC_RGBA8_4V1;
			case TextureFormat::BC1_RGB:
				return filament::backend::CompressedPixelDataType::DXT1_RGB;
			case TextureFormat::BC1_RGBA:
				return filament::backend::CompressedPixelDataType::DXT1_RGBA;
			case TextureFormat::BC2:
				return filament::backend::CompressedPixelDataType::DXT3_RGBA;
			case TextureFormat::BC3:
				return filament::backend::CompressedPixelDataType::DXT5_RGBA;
			case TextureFormat::ETC2_R8G8B8:
				return filament::backend::CompressedPixelDataType::ETC2_RGB8;
			case TextureFormat::ETC2_R8G8B8A1:
				return filament::backend::CompressedPixelDataType::ETC2_RGB8_A1;
			case TextureFormat::ETC2_R8G8B8A8:
				return filament::backend::CompressedPixelDataType::ETC2_EAC_RGBA8;
            case TextureFormat::ASTC_4x4:
                return filament::backend::CompressedPixelDataType::RGBA_ASTC_4x4;
			default:
				return filament::backend::CompressedPixelDataType::ETC1_RGB8;
		}
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

		texture->UpdateSampler(false);

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

		texture->UpdateSampler(false);

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
		bool depth = false;

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
				depth = true;
				break;
			case TextureFormat::D24S8:
			case TextureFormat::D32S8:
				usage |= filament::backend::TextureUsage::DEPTH_ATTACHMENT;
				usage |= filament::backend::TextureUsage::STENCIL_ATTACHMENT;
				depth = true;
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
		texture->m_filter_mode = depth ? FilterMode::Nearest : filter_mode;
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

		texture->UpdateSampler(depth);

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
        if (IsCompressedFormat(m_format))
        {
            auto data = filament::backend::PixelBufferDescriptor(
                buffer,
                pixels.Size(),
                GetCompressedPixelDataType(m_format),
                pixels.Size(),
                FreeBufferCallback);
            driver.updateCubeImage(m_texture, level, std::move(data), offsets);
        }
        else
        {
            auto data = filament::backend::PixelBufferDescriptor(
                buffer,
                pixels.Size(),
                GetPixelDataFormat(m_format),
                GetPixelDataType(m_format),
                FreeBufferCallback);
            driver.updateCubeImage(m_texture, level, std::move(data), offsets);
        }
	}

	void Texture::UpdateTexture(const ByteBuffer& pixels, int layer, int level, int x, int y, int w, int h)
	{
		auto& driver = Engine::Instance()->GetDriverApi();

		void* buffer = Memory::Alloc<void>(pixels.Size());
		Memory::Copy(buffer, pixels.Bytes(), pixels.Size());
		if (IsCompressedFormat(m_format))
		{
			auto data = filament::backend::PixelBufferDescriptor(
				buffer,
				pixels.Size(),
				GetCompressedPixelDataType(m_format),
				pixels.Size(),
				FreeBufferCallback);
			driver.updateTexture(m_texture, layer, level, x, y, w, h, std::move(data));
		}
		else
		{
			auto data = filament::backend::PixelBufferDescriptor(
				buffer,
				pixels.Size(),
				GetPixelDataFormat(m_format),
				GetPixelDataType(m_format),
				FreeBufferCallback);
			driver.updateTexture(m_texture, layer, level, x, y, w, h, std::move(data));
		}
	}

	void Texture::CopyTexture(
		int dst_layer, int dst_level,
		int dst_x, int dst_y,
		int dst_w, int dst_h,
		const Ref<Texture>& src,
		int src_layer, int src_level,
		int src_x, int src_y,
		int src_w, int src_h,
		FilterMode blit_filter)
	{
		auto& driver = Engine::Instance()->GetDriverApi();

		driver.copyTexture(
			m_texture, dst_layer, dst_level,
			filament::backend::Offset3D({ dst_x, dst_y, 0 }),
			filament::backend::Offset3D({ dst_w, dst_h, 1 }),
			src->m_texture, src_layer, src_level,
			filament::backend::Offset3D({ src_x, src_y, 0 }),
			filament::backend::Offset3D({ src_w, src_h, 1 }),
			blit_filter == FilterMode::Linear ? filament::backend::SamplerMagFilter::LINEAR : filament::backend::SamplerMagFilter::NEAREST);
	}

	void Texture::CopyToMemory(
		ByteBuffer& pixels,
		int layer, int level,
		int x, int y,
		int w, int h,
		std::function<void(const ByteBuffer&)> on_complete)
	{
		auto& driver = Engine::Instance()->GetDriverApi();

		auto data = filament::backend::PixelBufferDescriptor(
			pixels.Bytes(),
			pixels.Size(),
			GetPixelDataFormat(m_format),
			GetPixelDataType(m_format));
		driver.copyTextureToMemory(
			m_texture,
			layer, level,
			filament::backend::Offset3D({ x, y, 0 }),
			filament::backend::Offset3D({ w, h, 1 }),
			std::move(data),
			[=](const filament::backend::PixelBufferDescriptor&) {
				if (on_complete)
				{
					on_complete(pixels);
				}
			});
	}

	void Texture::GenMipmaps()
	{
		auto& driver = Engine::Instance()->GetDriverApi();

		if (driver.canGenerateMipmaps())
		{
			driver.generateMipmaps(m_texture);
		}
	}

	void Texture::UpdateSampler(bool depth)
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

		m_sampler.depthStencil = depth;
	}
}
