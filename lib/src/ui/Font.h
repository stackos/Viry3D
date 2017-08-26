#pragma once

#include "Object.h"

#define TEXTURE_SIZE_MAX 2048

namespace Viry3D
{
	struct GlyphInfo
	{
		char32_t c;
		int size;
		unsigned int glyph_index;
		int uv_pixel_x;
		int uv_pixel_y;
		int uv_pixel_w;
		int uv_pixel_h;
		int bearing_x;
		int bearing_y;
		int advance_x;
		int advance_y;
		bool bold;
		bool italic;
		bool mono;
	};

	class Texture2D;

	class Font : public Object
	{
	public:
		static void Init();
		static void Deinit();
		static Ref<Font> LoadFromFile(String file);
		~Font();
		void* GetFont() const { return m_font; }
		GlyphInfo GetGlyph(char32_t c, int size, bool bold, bool italic, bool mono);
		const Ref<Texture2D>& GetTexture() const { return m_texture; }

	private:
		Font();

		void* m_font;
		Map<char32_t, Map<int, GlyphInfo>> m_glyphs;
		Ref<Texture2D> m_texture;
		int m_texture_x;
		int m_texture_y;
		int m_texture_line_h_max;
	};
}