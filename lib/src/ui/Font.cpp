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

#include "Font.h"
#include "io/File.h"
#include "memory/Memory.h"
#include "graphics/Texture.h"
#include "Debug.h"
#include "Application.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "ftoutln.h"

extern "C"
{
    int z_verbose;
    void z_error(char* m) { }
}

namespace Viry3D
{
	static FT_Library g_ft_lib;
    Map<FontType, Ref<Font>> Font::m_fonts;

	void Font::Init()
	{
		FT_Init_FreeType(&g_ft_lib);
	}

	void Font::Done()
	{
        m_fonts.Clear();

		FT_Done_FreeType(g_ft_lib);
	}

    Ref<Font> Font::GetFont(FontType type)
    {
        Ref<Font> font;

        Ref<Font>* font_ptr;
        if (m_fonts.TryGet(type, &font_ptr))
        {
            font = *font_ptr;
        }
        else
        {
            String file;

            switch (type)
            {
            case FontType::Arial:
                file = "Arial.ttf";
                break;
            case FontType::Consola:
                file = "Consola.ttf";
                break;
            case FontType::PingFangSC:
                file = "PingFangSC.ttf";
                break;
            case FontType::SimSun:
                file = "SimSun.ttc";
                break;
            }

            font = Font::LoadFromFile(Application::Instance()->GetDataPath() + "/font/" + file);

            m_fonts.Add(type, font);
        }

        return font;
    }

	Ref<Font> Font::LoadFromFile(const String& file)
	{
		Ref<Font> font;

		if (File::Exist(file))
		{
            auto buffer = File::ReadAllBytes(file);

            FT_Face face;
            auto err = FT_New_Memory_Face(g_ft_lib, buffer.Bytes(), buffer.Size(), 0, &face);
			if (!err)
			{
				font = Ref<Font>(new Font());
				font->m_font = (void*) face;
                font->m_face_buffer = buffer;
			}
		}
        else
        {
            Log("Font::LoadFromFile font file not exist: %s", file.CString());
        }

		return font;
	}

	Font::Font():
		m_font(nullptr)
	{

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
        FT_Set_Char_Size(face, size << 6, size << 6, 0, 0);

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
        p_glyph->witdh = slot->bitmap.width;
        p_glyph->height = slot->bitmap.rows;
		p_glyph->bearing_x = slot->bitmap_left;
		p_glyph->bearing_y = slot->bitmap_top;
		p_glyph->advance_x = (int) (slot->advance.x >> 6);
		p_glyph->advance_y = (int) (slot->advance.y >> 6);

        if (p_glyph->witdh > 0 && p_glyph->height > 0)
        {
            ByteBuffer pixels = ByteBuffer(p_glyph->witdh * p_glyph->height * 4);

            if (mono)
            {
                for (int i = 0; i < p_glyph->height; ++i)
                {
                    for (int j = 0; j < p_glyph->witdh; ++j)
                    {
                        unsigned char bit = slot->bitmap.buffer[i * slot->bitmap.pitch + j / 8] & (0x1 << (7 - j % 8));
                        bit = bit == 0 ? 0 : 255;

                        pixels[i * p_glyph->witdh * 4 + j * 4 + 0] = 255;
                        pixels[i * p_glyph->witdh * 4 + j * 4 + 1] = 255;
                        pixels[i * p_glyph->witdh * 4 + j * 4 + 2] = 255;
                        pixels[i * p_glyph->witdh * 4 + j * 4 + 3] = bit;
                    }
                }
            }
            else
            {
                for (int i = 0; i < p_glyph->height; ++i)
                {
                    for (int j = 0; j < p_glyph->witdh; ++j)
                    {
                        unsigned char alpha = slot->bitmap.buffer[i * slot->bitmap.pitch + j];

                        pixels[i * p_glyph->witdh * 4 + j * 4 + 0] = 255;
                        pixels[i * p_glyph->witdh * 4 + j * 4 + 1] = 255;
                        pixels[i * p_glyph->witdh * 4 + j * 4 + 2] = 255;
                        pixels[i * p_glyph->witdh * 4 + j * 4 + 3] = alpha;
                    }
                }
            }

            p_glyph->texture = Texture::CreateTexture2DFromMemory(
                pixels,
                p_glyph->witdh,
                p_glyph->height,
                TextureFormat::R8G8B8A8,
                FilterMode::Linear,
                SamplerAddressMode::ClampToEdge,
                false,
                false,
                false);

            char32_t buffer[2] = { c, 0 };
            p_glyph->texture->SetName(String(buffer));
        }

		return *p_glyph;
	}

    bool Font::HasKerning() const
    {
        FT_Face face = (FT_Face) m_font;
        return FT_HAS_KERNING(face);
    }

    Vector2i Font::GetKerning(unsigned int previous_glyph_index, unsigned int glyph_index)
    {
        FT_Face face = (FT_Face) m_font;

        FT_Vector kerning;
        FT_Get_Kerning(face, previous_glyph_index, glyph_index, FT_KERNING_UNFITTED, &kerning);

        return Vector2i((int) kerning.x >> 6, (int) kerning.x >> 6);
    }
}
