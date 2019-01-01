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

#include "View.h"
#include "math/Vector2i.h"

namespace Viry3D
{
    enum class FontStyle
    {
        Normal,
        Bold,
        Italic,
        BoldAndItalic
    };

    struct CharMesh
    {
        char32_t c;
        Vector<Vector2i> vertices;
        Vector<Vector2> uv;
        Vector<Color> colors;
        Vector<unsigned short> indices;
        Ref<Texture> texture;
        Bounds bound;
    };

    struct LabelLine
    {
        int width;
        int height;
        Vector<CharMesh> meshes;

        void Clear()
        {
            width = 0;
            height = 0;
            meshes.Clear();
        }
    };

    class Font;

    class Label : public View
    {
    public:
        Label();
        virtual ~Label();
        virtual void UpdateLayout();
        void SetFont(const Ref<Font>& font);
        void SetFontStyle(FontStyle style);
        void SetFontSize(int size);
        const String& GetText() const { return m_text; }
        void SetText(const String& text);
        void SetLineSpace(int space);
        void SetRich(bool rich);
        void SetMono(bool mono);
        // use ViewAlignment
        void SetTextAlignment(int alignment);

    protected:
        virtual void FillSelfMeshes(Vector<ViewMesh>& meshes);

    private:
        void ProcessText();
        Vector2i ApplyTextAlignment(const Vector2i& target_size);

    private:
        Ref<Font> m_font;
        FontStyle m_font_style;
        int m_font_size;
        String m_text;
        int m_line_space;
        bool m_rich;
        bool m_mono;
        int m_text_alignment;
        Vector<LabelLine> m_lines;
        Vector2i m_content_size;
    };
}
