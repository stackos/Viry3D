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

#pragma once

#include "memory/Ref.h"
#include "container/Map.h"
#include "string/String.h"
#include "math/Vector2i.h"

namespace Viry3D
{
    enum class FontType
    {
        Arial,
        Consola,
        PingFangSC,
        SimSun,
    };

    class Texture;

	struct GlyphInfo
	{
		char32_t c;
		int size;
		unsigned int glyph_index;
        int witdh;
        int height;
		int bearing_x;
		int bearing_y;
		int advance_x;
		int advance_y;
		bool bold;
		bool italic;
		bool mono;
        Ref<Texture> texture;
	};

	class Font
	{
	public:
		static void Init();
		static void Done();
        static Ref<Font> GetFont(FontType type);
		static Ref<Font> LoadFromFile(const String& file);
		~Font();
		GlyphInfo GetGlyph(char32_t c, int size, bool bold, bool italic, bool mono);
        bool HasKerning() const;
        Vector2i GetKerning(unsigned int previous_glyph_index, unsigned int glyph_index);

	private:
		Font();

    private:
        static Map<FontType, Ref<Font>> m_fonts;
		void* m_font;
        ByteBuffer m_face_buffer;
		Map<char32_t, Map<int, GlyphInfo>> m_glyphs;
	};
}
