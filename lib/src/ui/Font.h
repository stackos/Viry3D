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

	class Font: public Object
	{
	public:
		static void Init();
		static void Deinit();
		static Ref<Font> LoadFromFile(const String& file);
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
