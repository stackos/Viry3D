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

#include "UILabel.h"
#include "graphics/Texture2D.h"
#include "graphics/Material.h"
#include "math/Mathf.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "ftoutln.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(UILabel);

	struct TagType
	{
		enum Enum
		{
			Color,
			Shadow,
			Outline,
			Underline,
			Bold,
			Italic
		};
	};

	struct TagInfo
	{
		String tag;
		TagType::Enum type;
		String value;
		int begin;
		int end;
	};

	UILabel::UILabel():
		m_font_style(FontStyle::Normal),
		m_font_size(20),
		m_line_space(1),
		m_rich(false),
		m_alignment(TextAlignment::UpperLeft)
	{
	}

	void UILabel::DeepCopy(const Ref<Object>& source)
	{
		UIView::DeepCopy(source);

		auto src = RefCast<UILabel>(source);
		m_font = src->m_font;
		m_font_style = src->m_font_style;
		m_font_size = src->m_font_size;
		m_text = src->m_text;
		m_line_space = src->m_line_space;
		m_rich = src->m_rich;
		m_alignment = src->m_alignment;
	}

	void UILabel::SetFont(const Ref<Font>& font)
	{
		if (m_font != font)
		{
			m_font = font;
			m_dirty = true;
			MarkRendererDirty();
		}
	}

	void UILabel::SetFontStyle(FontStyle::Enum style)
	{
		if (m_font_style != style)
		{
			m_font_style = style;
			m_dirty = true;
			MarkRendererDirty();
		}
	}

	void UILabel::SetFontSize(int size)
	{
		if (m_font_size != size)
		{
			m_font_size = size;
			m_dirty = true;
			MarkRendererDirty();
		}
	}

	void UILabel::SetText(const String& text)
	{
		if (m_text != text)
		{
			m_text = text;
			m_dirty = true;
			MarkRendererDirty();
		}
	}

	void UILabel::SetLineSpace(int space)
	{
		if (m_line_space != space)
		{
			m_line_space = space;
			m_dirty = true;
			MarkRendererDirty();
		}
	}

	void UILabel::SetRich(bool rich)
	{
		if (m_rich != rich)
		{
			m_rich = rich;
			m_dirty = true;
			MarkRendererDirty();
		}
	}

	void UILabel::SetAlignment(TextAlignment::Enum alignment)
	{
		if (m_alignment != alignment)
		{
			m_alignment = alignment;
			m_dirty = true;
			MarkRendererDirty();
		}
	}

	static bool check_tag_begin(Vector<char32_t>& str, int& char_index, const String& tag_str, int value_length, TagInfo& tag)
	{
		bool match = true;
		auto tag_cstr = tag_str.CString();

		for (int i = 0; i < tag_str.Size(); i++)
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
				for (int i = 0; i < value_length; i++)
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

	static bool check_tag_end(Vector<char32_t>& str, int& char_index, const String& tag_str, Vector<TagInfo>& tag_find, Vector<TagInfo>& tags)
	{
		bool match = true;
		auto tag_cstr = tag_str.CString();

		for (int i = 0; i < tag_str.Size(); i++)
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

			for (int i = tag_find.Size() - 1; i >= 0; i--)
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

	static Vector<TagInfo> parse_rich_tags(Vector<char32_t>& str)
	{
		Vector<TagInfo> tags;
		Vector<TagInfo> tag_find;

		for (int i = 0; i < str.Size(); i++)
		{
			TagInfo tag;

			if (check_tag_begin(str, i, TAG_COLOR_BEGIN, 8, tag))
			{
				tag.type = TagType::Color;
				tag_find.Add(tag);
			}
			else if (check_tag_end(str, i, TAG_COLOR_END, tag_find, tags))
			{
			}
			else if (check_tag_begin(str, i, TAG_SHADOW_BEGIN, 0, tag))
			{
				tag.type = TagType::Shadow;
				tag.value = "000000ff";
				tag_find.Add(tag);
			}
			else if (check_tag_begin(str, i, TAG_SHADOW_VALUE_BEGIN, 8, tag))
			{
				tag.type = TagType::Shadow;
				tag_find.Add(tag);
			}
			else if (check_tag_end(str, i, TAG_SHADOW_END, tag_find, tags))
			{
			}
			else if (check_tag_begin(str, i, TAG_OUTLINE_BEGIN, 0, tag))
			{
				tag.type = TagType::Outline;
				tag.value = "000000ff";
				tag_find.Add(tag);
			}
			else if (check_tag_begin(str, i, TAG_OUTLINE_VALUE_BEGIN, 8, tag))
			{
				tag.type = TagType::Outline;
				tag_find.Add(tag);
			}
			else if (check_tag_end(str, i, TAG_OUTLINE_END, tag_find, tags))
			{
			}
			else if (check_tag_begin(str, i, TAG_UNDERLINE_BEGIN, 0, tag))
			{
				tag.type = TagType::Underline;
				tag_find.Add(tag);
			}
			else if (check_tag_end(str, i, TAG_UNDERLINE_END, tag_find, tags))
			{
			}
			else if (check_tag_begin(str, i, TAG_BOLD_BEGIN, 0, tag))
			{
				tag.type = TagType::Bold;
				tag_find.Add(tag);
			}
			else if (check_tag_end(str, i, TAG_BOLD_END, tag_find, tags))
			{
			}
			else if (check_tag_begin(str, i, TAG_ITALIC_BEGIN, 0, tag))
			{
				tag.type = TagType::Italic;
				tag_find.Add(tag);
			}
			else if (check_tag_end(str, i, TAG_ITALIC_END, tag_find, tags))
			{
			}
		}

		return tags;
	}

	static Color string_to_color(String str)
	{
		str = str.ToLower();

		std::stringstream ss;
		ss << std::hex << str.CString();
		unsigned int color_i = 0;
		ss >> color_i;

		int r = (color_i & 0xff000000) >> 24;
		int g = (color_i & 0xff0000) >> 16;
		int b = (color_i & 0xff00) >> 8;
		int a = (color_i & 0xff);

		float div = 1 / 255.f;
		return Color((float) r, (float) g, (float) b, (float) a) * div;
	}

	Vector<LabelLine> UILabel::ProcessText(int& actual_width, int& actual_height)
	{
		auto chars = m_text.ToUnicode32();

		Vector<TagInfo> tags;
		if (m_rich)
		{
			tags = parse_rich_tags(chars);
		}

		auto face = (FT_Face) m_font->GetFont();
		auto label_size = GetSize();
		float v_size = 1.0f / TEXTURE_SIZE_MAX;
		int vertex_count = 0;
		auto has_kerning = FT_HAS_KERNING(face);
		FT_UInt previous = 0;
		int pen_x = 0;
		int pen_y = 0;
		int x_max = 0;
		int y_min = 0;
		int y_max = INT_MIN;
		int line_x_max = 0;
		int line_y_min = 0;
		Vector<LabelLine> lines;
		static LabelLine line;

		for (int i = 0; i < chars.Size(); i++)
		{
			char32_t c = chars[i];

			int font_size = m_font_size;
			Color color = m_color;
			bool bold = m_font_style == FontStyle::Bold || m_font_style == FontStyle::BoldAndItalic;
			bool italic = m_font_style == FontStyle::Italic || m_font_style == FontStyle::BoldAndItalic;
			bool mono = font_size <= 16;
			Ref<Color> color_shadow;
			Ref<Color> color_outline;
			bool underline = false;

			if (c == '\n')
			{
				line.width = line_x_max;
				line.height = pen_y - line_y_min;
				line_x_max = 0;
				line_y_min = 0;
				pen_x = 0;
				pen_y += -(font_size + m_line_space);

				lines.Add(line);
				line.Clear();

				continue;
			}

			if (m_rich)
			{
				for (auto& j : tags)
				{
					if (i >= j.begin && i < j.end)
					{
						switch (j.type)
						{
							case TagType::Color:
								color = string_to_color(j.value);
								break;
							case TagType::Bold:
								bold = true;
								break;
							case TagType::Italic:
								italic = true;
								break;
							case TagType::Shadow:
								color_shadow = RefMake<Color>(string_to_color(j.value));
								break;
							case TagType::Outline:
								color_outline = RefMake<Color>(string_to_color(j.value));
								break;
							case TagType::Underline:
								underline = true;
								break;
						}
					}
				}
			}

			GlyphInfo info = m_font->GetGlyph(c, font_size, bold, italic, mono);

			//	limit width
			if (pen_x + info.bearing_x + info.uv_pixel_w > label_size.x)
			{
				pen_x = 0;
				pen_y += -(font_size + m_line_space);
				previous = 0;
			}

			//	kerning
			if (has_kerning && previous && info.glyph_index)
			{
				FT_Vector delta;
				FT_Get_Kerning(face, previous, info.glyph_index, FT_KERNING_UNFITTED, &delta);
				pen_x += delta.x >> 6;
			}

			auto base_info = m_font->GetGlyph('A', font_size, bold, italic, mono);
			int base_y0 = base_info.bearing_y;
			int base_y1 = base_info.bearing_y - base_info.uv_pixel_h;
			int baseline = Mathf::RoundToInt(base_y0 + (font_size - base_y0 + base_y1) * 0.5f);

			int x0 = pen_x + info.bearing_x;
			int y0 = pen_y + info.bearing_y - baseline;
			int x1 = x0 + info.uv_pixel_w;
			int y1 = y0 - info.uv_pixel_h;

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

			int char_space = 0;
			pen_x += info.advance_x + char_space;

			int uv_x0 = info.uv_pixel_x;
			int uv_y0 = info.uv_pixel_y;
			int uv_x1 = uv_x0 + info.uv_pixel_w;
			int uv_y1 = uv_y0 + info.uv_pixel_h;

			if (color_shadow)
			{
				Vector2 offset = Vector2(1, -1);

				line.vertices.Add(Vector2((float) x0, (float) y0) + offset);
				line.vertices.Add(Vector2((float) x0, (float) y1) + offset);
				line.vertices.Add(Vector2((float) x1, (float) y1) + offset);
				line.vertices.Add(Vector2((float) x1, (float) y0) + offset);
				line.uv.Add(Vector2(uv_x0 * v_size, uv_y0 * v_size));
				line.uv.Add(Vector2(uv_x0 * v_size, uv_y1 * v_size));
				line.uv.Add(Vector2(uv_x1 * v_size, uv_y1 * v_size));
				line.uv.Add(Vector2(uv_x1 * v_size, uv_y0 * v_size));
				line.colors.Add(*color_shadow);
				line.colors.Add(*color_shadow);
				line.colors.Add(*color_shadow);
				line.colors.Add(*color_shadow);
				line.indices.Add(vertex_count + 0);
				line.indices.Add(vertex_count + 1);
				line.indices.Add(vertex_count + 2);
				line.indices.Add(vertex_count + 0);
				line.indices.Add(vertex_count + 2);
				line.indices.Add(vertex_count + 3);

				vertex_count += 4;
			}

			if (color_outline)
			{
				Vector2 offsets[4];
				offsets[0] = Vector2(-1, 1);
				offsets[1] = Vector2(-1, -1);
				offsets[2] = Vector2(1, -1);
				offsets[3] = Vector2(1, 1);

				for (int j = 0; j < 4; j++)
				{
					line.vertices.Add(Vector2((float) x0, (float) y0) + offsets[j]);
					line.vertices.Add(Vector2((float) x0, (float) y1) + offsets[j]);
					line.vertices.Add(Vector2((float) x1, (float) y1) + offsets[j]);
					line.vertices.Add(Vector2((float) x1, (float) y0) + offsets[j]);
					line.uv.Add(Vector2(uv_x0 * v_size, uv_y0 * v_size));
					line.uv.Add(Vector2(uv_x0 * v_size, uv_y1 * v_size));
					line.uv.Add(Vector2(uv_x1 * v_size, uv_y1 * v_size));
					line.uv.Add(Vector2(uv_x1 * v_size, uv_y0 * v_size));
					line.colors.Add(*color_outline);
					line.colors.Add(*color_outline);
					line.colors.Add(*color_outline);
					line.colors.Add(*color_outline);
					line.indices.Add(vertex_count + 0);
					line.indices.Add(vertex_count + 1);
					line.indices.Add(vertex_count + 2);
					line.indices.Add(vertex_count + 0);
					line.indices.Add(vertex_count + 2);
					line.indices.Add(vertex_count + 3);

					vertex_count += 4;
				}
			}

			line.vertices.Add(Vector2((float) x0, (float) y0));
			line.vertices.Add(Vector2((float) x0, (float) y1));
			line.vertices.Add(Vector2((float) x1, (float) y1));
			line.vertices.Add(Vector2((float) x1, (float) y0));
			line.uv.Add(Vector2(uv_x0 * v_size, uv_y0 * v_size));
			line.uv.Add(Vector2(uv_x0 * v_size, uv_y1 * v_size));
			line.uv.Add(Vector2(uv_x1 * v_size, uv_y1 * v_size));
			line.uv.Add(Vector2(uv_x1 * v_size, uv_y0 * v_size));
			line.colors.Add(color);
			line.colors.Add(color);
			line.colors.Add(color);
			line.colors.Add(color);
			line.indices.Add(vertex_count + 0);
			line.indices.Add(vertex_count + 1);
			line.indices.Add(vertex_count + 2);
			line.indices.Add(vertex_count + 0);
			line.indices.Add(vertex_count + 2);
			line.indices.Add(vertex_count + 3);

			vertex_count += 4;
			previous = info.glyph_index;

			if (underline)
			{
				int ux0 = pen_x - (info.advance_x + char_space);
				int uy0 = pen_y - baseline - 2;
				int ux1 = ux0 + info.advance_x + char_space;
				int uy1 = uy0 - 1;

				line.vertices.Add(Vector2((float) ux0, (float) uy0));
				line.vertices.Add(Vector2((float) ux0, (float) uy1));
				line.vertices.Add(Vector2((float) ux1, (float) uy1));
				line.vertices.Add(Vector2((float) ux1, (float) uy0));
				line.uv.Add(Vector2(0 * v_size, 0 * v_size));
				line.uv.Add(Vector2(0 * v_size, 1 * v_size));
				line.uv.Add(Vector2(1 * v_size, 1 * v_size));
				line.uv.Add(Vector2(1 * v_size, 0 * v_size));
				line.colors.Add(color);
				line.colors.Add(color);
				line.colors.Add(color);
				line.colors.Add(color);
				line.indices.Add(vertex_count + 0);
				line.indices.Add(vertex_count + 1);
				line.indices.Add(vertex_count + 2);
				line.indices.Add(vertex_count + 0);
				line.indices.Add(vertex_count + 2);
				line.indices.Add(vertex_count + 3);

				vertex_count += 4;
			}
		}

		//	最后一行
		if (!line.vertices.Empty())
		{
			line.width = line_x_max;
			line.height = pen_y - line_y_min;

			lines.Add(line);
			line.Clear();
		}

		actual_width = x_max;
		actual_height = -y_min;

		return lines;
	}

	void UILabel::FillVertices(Vector<Vector3>& vertices, Vector<Vector2>& uv, Vector<Color>& colors, Vector<unsigned short>& indices)
	{
		if (!m_font)
		{
			return;
		}

		Vector2 size = this->GetSize();
		Vector2 min = Vector2(-m_pivot.x * size.x, -m_pivot.y * size.y);
		Vector2 max = Vector2((1 - m_pivot.x) * size.x, (1 - m_pivot.y) * size.y);

		int actual_width;
		int actual_height;
		auto lines = this->ProcessText(actual_width, actual_height);

		auto mat = GetVertexMatrix();
		int index_begin = vertices.Size();

		for (int i = 0; i < lines.Size(); i++)
		{
			auto line = lines[i];

			for (int j = 0; j < line.vertices.Size(); j++)
			{
				auto v = line.vertices[j];

				switch (m_alignment)
				{
					case TextAlignment::UpperLeft:
					case TextAlignment::MiddleLeft:
					case TextAlignment::LowerLeft:
						v.x += min.x;
						break;
					case TextAlignment::UpperCenter:
					case TextAlignment::MiddleCenter:
					case TextAlignment::LowerCenter:
						v.x += min.x + (size.x - line.width) / 2;
						break;
					case TextAlignment::UpperRight:
					case TextAlignment::MiddleRight:
					case TextAlignment::LowerRight:
						v.x += min.x + (size.x - line.width);
						break;
				}

				switch (m_alignment)
				{
					case TextAlignment::UpperLeft:
					case TextAlignment::UpperCenter:
					case TextAlignment::UpperRight:
						v.y += max.y;
						break;
					case TextAlignment::MiddleLeft:
					case TextAlignment::MiddleCenter:
					case TextAlignment::MiddleRight:
						v.y += max.y - (size.y - actual_height) / 2;
						break;
					case TextAlignment::LowerLeft:
					case TextAlignment::LowerCenter:
					case TextAlignment::LowerRight:
						v.y += max.y - (size.y - actual_height);
						break;
				}

				vertices.Add(mat.MultiplyPoint3x4(v));
			}

			if (!line.vertices.Empty())
			{
				uv.AddRange(&line.uv[0], line.uv.Size());
				colors.AddRange(&line.colors[0], line.colors.Size());
			}

			for (int j = 0; j < line.indices.Size(); j++)
			{
				indices.Add(line.indices[j] + index_begin);
			}
		}
	}

	void UILabel::FillMaterial(Ref<Material>& mat)
	{
		if (m_font)
		{
			mat->SetMainTexture(m_font->GetTexture());
		}
	}
}
