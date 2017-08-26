#pragma once

#include "Texture.h"
#include "TextureFormat.h"

namespace Viry3D
{
	class Texture2D : public Texture
	{
	public:
		//
		//	线程安全, 可在子线程中异步加载
		//
		static Ref<Texture2D> LoadFromFile(String file,
			TextureWrapMode::Enum wrap_mode = TextureWrapMode::Clamp,
			FilterMode::Enum filter_mode = FilterMode::Bilinear,
			bool mipmap = false);
		//
		//	线程安全, 可在子线程中异步加载
		//
		static Ref<Texture2D> LoadFromData(const ByteBuffer& buffer,
			TextureWrapMode::Enum wrap_mode = TextureWrapMode::Clamp,
			FilterMode::Enum filter_mode = FilterMode::Bilinear,
			bool mipmap = false);
		//
		//	线程安全
		//
		static Ref<Texture2D> Create(
			int width,
			int height,
			TextureFormat::Enum format,
			TextureWrapMode::Enum wrap_mode,
			FilterMode::Enum filter_mode,
			bool mipmap,
			const ByteBuffer& colors);

		ByteBuffer GetColors() const { return m_colors; }
		void UpdateTexture(int x, int y, int w, int h, const ByteBuffer& colors);
		void EncodeToPNG(String file);
		TextureFormat::Enum GetFormat() const { return m_format; }
		bool IsMipmap() const { return m_mipmap; }

	private:
		Texture2D();
		void SetFormat(TextureFormat::Enum format) { m_format = format; }
		void SetMipmap(bool mipmap) { m_mipmap = mipmap; }

	private:
		TextureFormat::Enum m_format;
		bool m_mipmap;
		ByteBuffer m_colors;
	};
}