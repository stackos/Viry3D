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
        Ref<Image> image;
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
        virtual void SetSize(const Vector2i& size);
        const Ref<Font>& GetFont() const { return m_font; }
        void SetFont(const Ref<Font>& font);
        FontStyle GetFontStyle() const { return m_font_style; }
        void SetFontStyle(FontStyle style);
        int GetFontSize() const { return m_font_size; }
        void SetFontSize(int size);
        const String& GetText() const { return m_text; }
        void SetText(const String& text);
        int GetLineSpace() const { return m_line_space; }
        void SetLineSpace(int space);
        bool IsRich() const { return m_rich; }
        void SetRich(bool rich);
        bool IsMono() const { return m_mono; }
        void SetMono(bool mono);
        int GetTextAlignment() const { return m_text_alignment; }
        // use ViewAlignment
        void SetTextAlignment(int alignment);
        bool IsWrapContent() const { return m_wrap_content; }
        void SetWrapContent(bool enable);
        const Vector<LabelLine>& GetLines();

    protected:
        virtual void FillSelfMeshes(Vector<ViewMesh>& meshes, const Rect& clip_rect);

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
        bool m_wrap_content;
        bool m_lines_dirty;
    };
}
