/*
* Viry3D
* Copyright 2014-2017 by Stack - stackos@qq.com
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

#include "Font.h"
#include "io/File.h"
#include "graphics/Texture2D.h"
#include "memory/Memory.h"
#include "Debug.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "ftoutln.h"

namespace Viry3D
{
	static FT_Library g_ft_lib;

	extern "C"
	{
		int z_verbose;
		void z_error(char *m) { }
	}

	void Font::Init()
	{
		FT_Init_FreeType(&g_ft_lib);
	}

	void Font::Deinit()
	{
		FT_Done_FreeType(g_ft_lib);
	}

	Ref<Font> Font::LoadFromFile(const String& file)
	{
		Ref<Font> font;

		if (File::Exist(file))
		{
			FT_Face face;
			auto err = FT_New_Face(g_ft_lib, file.CString(), 0, &face);
			if (!err)
			{
				font = Ref<Font>(new Font());
				font->m_font = (void*) face;
			}
		}

		return font;
	}

	Font::Font():
		m_font(NULL),
		m_texture_x(0),
		m_texture_y(0),
		m_texture_line_h_max(0)
	{
		auto buffer = ByteBuffer(TEXTURE_SIZE_MAX * TEXTURE_SIZE_MAX);
		Memory::Zero(buffer.Bytes(), buffer.Size());
		m_texture = Texture2D::Create(
			TEXTURE_SIZE_MAX, TEXTURE_SIZE_MAX,
			TextureFormat::R8,
			TextureWrapMode::Clamp, FilterMode::Point,
			false,
			buffer);
	}

	Font::~Font()
	{
		if (m_font)
		{
			FT_Done_Face((FT_Face) m_font);
		}
	}

	GlyphInfo Font::GetGlyph(char32_t c, int size, bool bold, bool italic, bool mono)
	{
		int size_key = size | (bold ? (1 << 24) : 0) | (italic ? (1 << 16) : 0);

		Map<int, GlyphInfo>* p_size_glyphs;
		if (!m_glyphs.TryGet(c, &p_size_glyphs))
		{
			Map<int, GlyphInfo> size_glyphs;
			m_glyphs.Add(c, size_glyphs);

			p_size_glyphs = &m_glyphs[c];
		}

		GlyphInfo* p_glyph;
		if (!p_size_glyphs->TryGet(size_key, &p_glyph))
		{
			GlyphInfo glyph;
			p_size_glyphs->Add(size_key, glyph);

			p_glyph = &(*p_size_glyphs)[size_key];
		}
		else
		{
			return *p_glyph;
		}

		p_glyph->c = c;
		p_glyph->size = size;
		p_glyph->bold = bold;
		p_glyph->italic = italic;
		p_glyph->mono = mono;

		FT_Face face = (FT_Face) m_font;
		FT_Set_Pixel_Sizes(face, 0, size);

		FT_GlyphSlot slot = face->glyph;
		auto glyph_index = FT_Get_Char_Index(face, c);

		if (mono)
		{
			FT_Load_Char(face, c, FT_LOAD_MONOCHROME | FT_LOAD_TARGET_MONO);
		}
		else
		{
			FT_Load_Char(face, c, FT_LOAD_DEFAULT);
		}

		if (bold)
		{
			FT_Outline_Embolden(&face->glyph->outline, 1 << 6);
		}

		if (italic)
		{
			float lean = 0.5f;
			FT_Matrix matrix;
			matrix.xx = 1 << 16;
			matrix.xy = (int) (lean * (1 << 16));
			matrix.yx = 0;
			matrix.yy = (1 << 16);
			FT_Outline_Transform(&face->glyph->outline, &matrix);
		}

		if (mono)
		{
			FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);
		}
		else
		{
			FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		}

		p_glyph->glyph_index = glyph_index;
		p_glyph->uv_pixel_w = slot->bitmap.width;
		p_glyph->uv_pixel_h = slot->bitmap.rows;
		p_glyph->bearing_x = slot->bitmap_left;
		p_glyph->bearing_y = slot->bitmap_top;
		p_glyph->advance_x = (int) (slot->advance.x >> 6);
		p_glyph->advance_y = (int) (slot->advance.y >> 6);

		if (m_texture_y + p_glyph->uv_pixel_h <= TEXTURE_SIZE_MAX)
		{
			auto colors = m_texture->GetColors();

			// insert one white pixel for underline
			if (m_texture_x == 0 && m_texture_y == 0)
			{
				ByteBuffer buffer(1);
				buffer[0] = 0xff;
				colors[0] = 0xff;
				m_texture->UpdateTexture(0, 0, 1, 1, buffer);
				m_texture_x += 1;
			}

			if (m_texture_x + p_glyph->uv_pixel_w > TEXTURE_SIZE_MAX)
			{
				m_texture_y += m_texture_line_h_max;
				m_texture_x = 0;
				m_texture_line_h_max = 0;
			}

			if (m_texture_line_h_max < p_glyph->uv_pixel_h)
			{
				m_texture_line_h_max = p_glyph->uv_pixel_h;
			}

			ByteBuffer char_pixels;

			if (mono)
			{
				char_pixels = ByteBuffer(p_glyph->uv_pixel_w * p_glyph->uv_pixel_h);

				for (int i = 0; i < p_glyph->uv_pixel_h; i++)
				{
					for (int j = 0; j < p_glyph->uv_pixel_w; j++)
					{
						unsigned char bit = slot->bitmap.buffer[i * slot->bitmap.pitch + j / 8] & (0x1 << (7 - j % 8));
						bit = bit == 0 ? 0 : 255;
						char_pixels[i * p_glyph->uv_pixel_w + j] = bit;
					}
				}
			}
			else
			{
				char_pixels = ByteBuffer(slot->bitmap.buffer, p_glyph->uv_pixel_w * p_glyph->uv_pixel_h);
			}

			for (int i = 0; i < p_glyph->uv_pixel_h; i++)
			{
				Memory::Copy(&colors[TEXTURE_SIZE_MAX * (m_texture_y + i) + m_texture_x], &char_pixels[p_glyph->uv_pixel_w * i], p_glyph->uv_pixel_w);
			}

			if (p_glyph->uv_pixel_w > 0 && p_glyph->uv_pixel_h > 0)
			{
				m_texture->UpdateTexture(m_texture_x, m_texture_y, p_glyph->uv_pixel_w, p_glyph->uv_pixel_h, char_pixels);
			}

			p_glyph->uv_pixel_x = m_texture_x;
			p_glyph->uv_pixel_y = m_texture_y;

			m_texture_x += p_glyph->uv_pixel_w;
		}
		else
		{
			Log("font texture size over than 2048");
		}

		return *p_glyph;
	}
}
