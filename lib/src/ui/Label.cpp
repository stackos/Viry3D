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

#include "Label.h"
#include "CanvasRenderer.h"
#include "Font.h"
#include "graphics/Texture.h"

namespace Viry3D
{
    enum class TagType
    {
        Color,
        Shadow,
        Outline,
        Underline,
        Bold,
        Italic
    };

    struct TagInfo
    {
        String tag;
        TagType type;
        String value;
        int begin;
        int end;
    };

    static bool CheckTagBegin(Vector<char32_t>& str, int& char_index, const String& tag_str, int value_length, TagInfo& tag)
    {
        bool match = true;
        auto tag_cstr = tag_str.CString();

        for (int i = 0; i < tag_str.Size(); ++i)
        {
            if (tag_cstr[i] != str[char_index + i])
            {
                match = false;
                break;
            }
        }

        if (match)
        {
            if (value_length > 0)
            {
                Vector<char32_t> value;
                for (int i = 0; i < value_length; ++i)
                {
                    value.Add(str[char_index + tag_str.Size() + i]);
                }
                value.Add(0);

                tag.tag = tag_str.Substring(1, tag_str.Size() - 3);
                tag.value = String(&value[0]);

                str.RemoveRange(char_index, tag_str.Size() + value_length + 1);
            }
            else
            {
                tag.tag = tag_str.Substring(1, tag_str.Size() - 2);

                str.RemoveRange(char_index, tag_str.Size());
            }

            tag.begin = char_index--;
        }

        return match;
    }

    static bool CheckTagEnd(Vector<char32_t>& str, int& char_index, const String& tag_str, Vector<TagInfo>& tag_find, Vector<TagInfo>& tags)
    {
        bool match = true;
        auto tag_cstr = tag_str.CString();

        for (int i = 0; i < tag_str.Size(); ++i)
        {
            if (tag_cstr[i] != str[char_index + i])
            {
                match = false;
                break;
            }
        }

        if (match)
        {
            auto tag = tag_str.Substring(2, tag_str.Size() - 3);

            for (int i = tag_find.Size() - 1; i >= 0; --i)
            {
                auto &t = tag_find[i];

                if (t.tag == tag)
                {
                    str.RemoveRange(char_index, tag_str.Size());
                    t.end = char_index--;
                    tags.Add(t);
                    tag_find.Remove(i);
                    break;
                }
            }
        }

        return match;
    }

    static const String TAG_COLOR_BEGIN = "<color=#";
    static const String TAG_COLOR_END = "</color>";
    static const String TAG_SHADOW_BEGIN = "<shadow>";
    static const String TAG_SHADOW_VALUE_BEGIN = "<shadow=#";
    static const String TAG_SHADOW_END = "</shadow>";
    static const String TAG_OUTLINE_BEGIN = "<outline>";
    static const String TAG_OUTLINE_VALUE_BEGIN = "<outline=#";
    static const String TAG_OUTLINE_END = "</outline>";
    static const String TAG_UNDERLINE_BEGIN = "<underline>";
    static const String TAG_UNDERLINE_END = "</underline>";
    static const String TAG_BOLD_BEGIN = "<bold>";
    static const String TAG_BOLD_END = "</bold>";
    static const String TAG_ITALIC_BEGIN = "<italic>";
    static const String TAG_ITALIC_END = "</italic>";

    static Vector<TagInfo> ParseRichTag(Vector<char32_t>& str)
    {
        Vector<TagInfo> tags;
        Vector<TagInfo> tag_find;

        for (int i = 0; i < str.Size(); ++i)
        {
            TagInfo tag;

            if (CheckTagBegin(str, i, TAG_COLOR_BEGIN, 8, tag))
            {
                tag.type = TagType::Color;
                tag_find.Add(tag);
            }
            else if (CheckTagEnd(str, i, TAG_COLOR_END, tag_find, tags))
            {
            }
            else if (CheckTagBegin(str, i, TAG_SHADOW_BEGIN, 0, tag))
            {
                tag.type = TagType::Shadow;
                tag.value = "000000ff";
                tag_find.Add(tag);
            }
            else if (CheckTagBegin(str, i, TAG_SHADOW_VALUE_BEGIN, 8, tag))
            {
                tag.type = TagType::Shadow;
                tag_find.Add(tag);
            }
            else if (CheckTagEnd(str, i, TAG_SHADOW_END, tag_find, tags))
            {
            }
            else if (CheckTagBegin(str, i, TAG_OUTLINE_BEGIN, 0, tag))
            {
                tag.type = TagType::Outline;
                tag.value = "000000ff";
                tag_find.Add(tag);
            }
            else if (CheckTagBegin(str, i, TAG_OUTLINE_VALUE_BEGIN, 8, tag))
            {
                tag.type = TagType::Outline;
                tag_find.Add(tag);
            }
            else if (CheckTagEnd(str, i, TAG_OUTLINE_END, tag_find, tags))
            {
            }
            else if (CheckTagBegin(str, i, TAG_UNDERLINE_BEGIN, 0, tag))
            {
                tag.type = TagType::Underline;
                tag_find.Add(tag);
            }
            else if (CheckTagEnd(str, i, TAG_UNDERLINE_END, tag_find, tags))
            {
            }
            else if (CheckTagBegin(str, i, TAG_BOLD_BEGIN, 0, tag))
            {
                tag.type = TagType::Bold;
                tag_find.Add(tag);
            }
            else if (CheckTagEnd(str, i, TAG_BOLD_END, tag_find, tags))
            {
            }
            else if (CheckTagBegin(str, i, TAG_ITALIC_BEGIN, 0, tag))
            {
                tag.type = TagType::Italic;
                tag_find.Add(tag);
            }
            else if (CheckTagEnd(str, i, TAG_ITALIC_END, tag_find, tags))
            {
            }
        }

        return tags;
    }

    static Color StringToColor(const String& str)
    {
        auto str_lower = str.ToLower();

        std::stringstream ss;
        ss << std::hex << str_lower.CString();
        unsigned int color_i = 0;
        ss >> color_i;

        int r = (color_i & 0xff000000) >> 24;
        int g = (color_i & 0xff0000) >> 16;
        int b = (color_i & 0xff00) >> 8;
        int a = (color_i & 0xff);

        float div = 1 / 255.f;
        return Color((float) r, (float) g, (float) b, (float) a) * div;
    }

    Label::Label():
        m_font_style(FontStyle::Normal),
        m_font_size(20),
        m_line_space(0),
        m_rich(false),
        m_mono(false),
        m_text_alignment(ViewAlignment::HCenter | ViewAlignment::VCenter)
    {
    
    }
    
    Label::~Label()
    {
    
    }

    void Label::SetFont(const Ref<Font>& font)
    {
        m_font = font;
        this->MarkCanvasDirty();
    }

    void Label::SetFontStyle(FontStyle style)
    {
        m_font_style = style;
        this->MarkCanvasDirty();
    }

    void Label::SetFontSize(int size)
    {
        m_font_size = size;
        this->MarkCanvasDirty();
    }

    void Label::SetText(const String& text)
    {
        if (m_text != text)
        {
            m_text = text;
            this->MarkCanvasDirty();
        }
    }

    void Label::SetLineSpace(int space)
    {
        m_line_space = space;
        this->MarkCanvasDirty();
    }

    void Label::SetRich(bool rich)
    {
        m_rich = rich;
        this->MarkCanvasDirty();
    }

    void Label::SetMono(bool mono)
    {
        m_mono = mono;
        this->MarkCanvasDirty();
    }

    void Label::SetTextAlignment(int alignment)
    {
        m_text_alignment = alignment;
        this->MarkCanvasDirty();
    }

    void Label::ProcessText()
    {
        m_lines.Clear();

        if (!m_font)
        {
            return;
        }

        auto chars = m_text.ToUnicode32();

        Vector<TagInfo> tags;
        if (m_rich)
        {
            tags = ParseRichTag(chars);
        }

        int pen_x = 0;
        int pen_y = 0;
        int x_max = 0;
        int y_min = 0;
        int y_max = INT_MIN;
        int line_x_max = 0;
        int line_y_min = 0;
        int font_size = m_font_size;
        bool mono = m_mono;
        bool has_kerning = m_font->HasKerning();
        unsigned int previous = 0;
        LabelLine line;

        for (int i = 0; i < chars.Size(); ++i)
        {
            char32_t c = chars[i];

            if (c == '\n')
            {
                line.width = line_x_max;
                line.height = pen_y - line_y_min;
                line_x_max = 0;
                line_y_min = 0;
                pen_x = 0;
                pen_y += -(font_size + m_line_space);

                m_lines.Add(line);
                line.Clear();

                continue;
            }

            Color color = this->GetColor();
            bool bold = m_font_style == FontStyle::Bold || m_font_style == FontStyle::BoldAndItalic;
            bool italic = m_font_style == FontStyle::Italic || m_font_style == FontStyle::BoldAndItalic;
            Ref<Color> color_shadow;
            Ref<Color> color_outline;
            bool underline = false;

            if (m_rich)
            {
                for (auto& j : tags)
                {
                    if (i >= j.begin && i < j.end)
                    {
                        switch (j.type)
                        {
                        case TagType::Color:
                            color = StringToColor(j.value);
                            break;
                        case TagType::Bold:
                            bold = true;
                            break;
                        case TagType::Italic:
                            italic = true;
                            break;
                        case TagType::Shadow:
                            color_shadow = RefMake<Color>(StringToColor(j.value));
                            break;
                        case TagType::Outline:
                            color_outline = RefMake<Color>(StringToColor(j.value));
                            break;
                        case TagType::Underline:
                            underline = true;
                            break;
                        }
                    }
                }
            }

            GlyphInfo info = m_font->GetGlyph(c, font_size, bold, italic, mono);

            //	kerning
            if (has_kerning && previous && info.glyph_index)
            {
                pen_x += m_font->GetKerning(previous, info.glyph_index).x;
            }

            auto base_info = m_font->GetGlyph('A', font_size, bold, italic, mono);
            int base_y0 = base_info.bearing_y;
            int base_y1 = base_info.bearing_y - base_info.height;
            int baseline = Mathf::RoundToInt(base_y0 + (font_size - base_y0 + base_y1) * 0.5f);
            const int char_space = 0;

            int x0 = pen_x + info.bearing_x;
            int y0 = pen_y + info.bearing_y - baseline;
            int x1 = x0 + info.witdh;
            if (c == ' ')
            {
                x1 = pen_x + info.advance_x + char_space;
            }
            int y1 = y0 - info.height;

            if (x_max < x1)
            {
                x_max = x1;
            }
            if (y_min > y1)
            {
                y_min = y1;
            }
            if (y_max < y0)
            {
                y_max = y0;
            }

            if (line_x_max < x1)
            {
                line_x_max = x1;
            }
            if (line_y_min > y1)
            {
                line_y_min = y1;
            }

            pen_x += info.advance_x + char_space;

            CharMesh mesh;
            mesh.c = c;
            mesh.texture = info.texture;
            mesh.bound = Bounds(Vector3((float) x0, (float) y1, 0), Vector3((float) x1, (float) y0, 0));

            float uv_x0 = 0;
            float uv_y0 = 0;
            float uv_x1 = 1;
            float uv_y1 = 1;
            int vertex_count = 0;

            if (color_shadow)
            {
                Vector2i offset = Vector2i(1, -1);

                mesh.vertices.Add(Vector2i(x0, y0) + offset);
                mesh.vertices.Add(Vector2i(x0, y1) + offset);
                mesh.vertices.Add(Vector2i(x1, y1) + offset);
                mesh.vertices.Add(Vector2i(x1, y0) + offset);
                mesh.uv.Add(Vector2(uv_x0, uv_y0));
                mesh.uv.Add(Vector2(uv_x0, uv_y1));
                mesh.uv.Add(Vector2(uv_x1, uv_y1));
                mesh.uv.Add(Vector2(uv_x1, uv_y0));
                mesh.colors.Add(*color_shadow);
                mesh.colors.Add(*color_shadow);
                mesh.colors.Add(*color_shadow);
                mesh.colors.Add(*color_shadow);
                mesh.indices.Add(vertex_count + 0);
                mesh.indices.Add(vertex_count + 1);
                mesh.indices.Add(vertex_count + 2);
                mesh.indices.Add(vertex_count + 0);
                mesh.indices.Add(vertex_count + 2);
                mesh.indices.Add(vertex_count + 3);

                vertex_count += 4;
            }

            if (color_outline)
            {
                Vector2i offsets[4];
                offsets[0] = Vector2i(-1, 1);
                offsets[1] = Vector2i(-1, -1);
                offsets[2] = Vector2i(1, -1);
                offsets[3] = Vector2i(1, 1);

                for (int j = 0; j < 4; ++j)
                {
                    mesh.vertices.Add(Vector2i(x0, y0) + offsets[j]);
                    mesh.vertices.Add(Vector2i(x0, y1) + offsets[j]);
                    mesh.vertices.Add(Vector2i(x1, y1) + offsets[j]);
                    mesh.vertices.Add(Vector2i(x1, y0) + offsets[j]);
                    mesh.uv.Add(Vector2(uv_x0, uv_y0));
                    mesh.uv.Add(Vector2(uv_x0, uv_y1));
                    mesh.uv.Add(Vector2(uv_x1, uv_y1));
                    mesh.uv.Add(Vector2(uv_x1, uv_y0));
                    mesh.colors.Add(*color_outline);
                    mesh.colors.Add(*color_outline);
                    mesh.colors.Add(*color_outline);
                    mesh.colors.Add(*color_outline);
                    mesh.indices.Add(vertex_count + 0);
                    mesh.indices.Add(vertex_count + 1);
                    mesh.indices.Add(vertex_count + 2);
                    mesh.indices.Add(vertex_count + 0);
                    mesh.indices.Add(vertex_count + 2);
                    mesh.indices.Add(vertex_count + 3);

                    vertex_count += 4;
                }
            }

            {
                mesh.vertices.Add(Vector2i(x0, y0));
                mesh.vertices.Add(Vector2i(x0, y1));
                mesh.vertices.Add(Vector2i(x1, y1));
                mesh.vertices.Add(Vector2i(x1, y0));
                mesh.uv.Add(Vector2(uv_x0, uv_y0));
                mesh.uv.Add(Vector2(uv_x0, uv_y1));
                mesh.uv.Add(Vector2(uv_x1, uv_y1));
                mesh.uv.Add(Vector2(uv_x1, uv_y0));
                mesh.colors.Add(color);
                mesh.colors.Add(color);
                mesh.colors.Add(color);
                mesh.colors.Add(color);
                mesh.indices.Add(vertex_count + 0);
                mesh.indices.Add(vertex_count + 1);
                mesh.indices.Add(vertex_count + 2);
                mesh.indices.Add(vertex_count + 0);
                mesh.indices.Add(vertex_count + 2);
                mesh.indices.Add(vertex_count + 3);

                vertex_count += 4;
            }

            previous = info.glyph_index;

            line.meshes.Add(mesh);

            if (underline)
            {
                CharMesh underline_mesh;
                underline_mesh.c = 0;
                underline_mesh.texture = Texture::GetSharedWhiteTexture();

                int ux0 = pen_x - (info.advance_x + char_space);
                int uy0 = pen_y - baseline - 2;
                int ux1 = ux0 + info.advance_x + char_space;
                int uy1 = uy0 - 1;

                underline_mesh.vertices.Add(Vector2i(ux0, uy0));
                underline_mesh.vertices.Add(Vector2i(ux0, uy1));
                underline_mesh.vertices.Add(Vector2i(ux1, uy1));
                underline_mesh.vertices.Add(Vector2i(ux1, uy0));
                underline_mesh.uv.Add(Vector2(1.0f / 3, 1.0f / 3));
                underline_mesh.uv.Add(Vector2(1.0f / 3, 2.0f / 3));
                underline_mesh.uv.Add(Vector2(2.0f / 3, 2.0f / 3));
                underline_mesh.uv.Add(Vector2(2.0f / 3, 1.0f / 3));
                underline_mesh.colors.Add(color);
                underline_mesh.colors.Add(color);
                underline_mesh.colors.Add(color);
                underline_mesh.colors.Add(color);
                underline_mesh.indices.Add(0);
                underline_mesh.indices.Add(1);
                underline_mesh.indices.Add(2);
                underline_mesh.indices.Add(0);
                underline_mesh.indices.Add(2);
                underline_mesh.indices.Add(3);

                line.meshes.Add(underline_mesh);
            }
        }

        if (line.meshes.Size() > 0)
        {
            line.width = line_x_max;
            line.height = pen_y - line_y_min;

            m_lines.Add(line);
        }

        m_content_size = Vector2i(x_max, -y_min);
    }

    void Label::UpdateLayout()
    {
        View::UpdateLayout();

        this->ProcessText();
    }

    Vector2i Label::ApplyTextAlignment(const Vector2i& target_size)
    {
        Vector2i offset_pos;

        if (m_text_alignment & ViewAlignment::Left)
        {
            offset_pos.x = 0;
        }
        else if (m_text_alignment & ViewAlignment::HCenter)
        {
            offset_pos.x = (int) (target_size.x / 2) - m_content_size.x / 2;
        }
        else if (m_text_alignment & ViewAlignment::Right)
        {
            offset_pos.x = (int) target_size.x - m_content_size.x;
        }

        if (m_text_alignment & ViewAlignment::Top)
        {
            offset_pos.y = 0;
        }
        else if (m_text_alignment & ViewAlignment::VCenter)
        {
            offset_pos.y = - (int) (target_size.y / 2) + m_content_size.y / 2;
        }
        else if (m_text_alignment & ViewAlignment::Bottom)
        {
            offset_pos.y = - (int) target_size.y + m_content_size.y;
        }

        return offset_pos;
    }

    void Label::FillSelfMeshes(Vector<ViewMesh>& meshes)
    {
        View::FillSelfMeshes(meshes);

        Rect rect;
        Matrix4x4 matrix;
        this->ComputeVerticesRectAndMatrix(rect, matrix);

        Vector2i offset_pos = this->ApplyTextAlignment(Vector2i((int) rect.width, (int) rect.height));

        for (int i = 0; i < m_lines.Size(); ++i)
        {
            const LabelLine& line = m_lines[i];
            
            for (int j = 0; j < line.meshes.Size(); ++j)
            {
                const CharMesh& char_mesh = line.meshes[j];

                ViewMesh mesh;
                mesh.vertices.Resize(char_mesh.vertices.Size());

                for (int k = 0; k < mesh.vertices.Size(); ++k)
                {
                    float x = rect.x + offset_pos.x + char_mesh.vertices[k].x;
                    float y = rect.y + offset_pos.y + char_mesh.vertices[k].y;

                    mesh.vertices[k].vertex = matrix.MultiplyPoint3x4(Vector3(x, y, 0));
                    mesh.vertices[k].uv = char_mesh.uv[k];
                    mesh.vertices[k].color = char_mesh.colors[k] * this->GetColor();
                }

                mesh.indices.AddRange(char_mesh.indices);
                mesh.texture = char_mesh.texture;
                mesh.view = this;
                mesh.base_view = false;

                meshes.Add(mesh);
            }
        }
    }
}
