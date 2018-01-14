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

#include "UIView.h"
#include "Font.h"
#include "math/Bounds.h"

namespace Viry3D
{
	enum class FontStyle
	{
		Normal,
		Bold,
		Italic,
		BoldAndItalic
	};

	enum class TextAlignment
	{
		UpperLeft,
		UpperCenter,
		UpperRight,
		MiddleLeft,
		MiddleCenter,
		MiddleRight,
		LowerLeft,
		LowerCenter,
		LowerRight
	};

    enum class HorizontalWrapMode
    {
        Wrap,
        Overflow,
    };

    enum class VerticalWrapMode
    {
        Truncate,
        Overflow,
    };

	struct LabelLine
	{
		int width;
		int height;
		Vector<Vector2> vertices;
		Vector<Vector2> uv;
		Vector<Color> colors;
		Vector<unsigned short> indices;
		Vector<char32_t> chars;
		Vector<Bounds> char_bounds;

		void Clear()
		{
			vertices.Clear();
			uv.Clear();
			colors.Clear();
			indices.Clear();
			chars.Clear();
			char_bounds.Clear();
		}
	};

	//
	//	Supported rich tags
	//
	//	<color=#ffffffff></color>
	//	<shadow></shadow>
	//	<shadow=#000000ff></shadow>
	//	<outline></outline>
	//	<outline=#000000ff></outline>
	//	<underline></underline>
	//	<bold></bold>£¨¥÷ÃÂ
	//	<italic></italic>£¨–±ÃÂ
	//
	class UILabel: public UIView
	{
		DECLARE_COM_CLASS(UILabel, UIView);
	public:
		void SetFont(const Ref<Font>& font);
		void SetFontStyle(FontStyle style);
		void SetFontSize(int size);
		void SetText(const String& text);
		void SetLineSpace(int space);
		void SetRich(bool rich);
		void SetMono(bool mono);
		void SetAlignment(TextAlignment alignment);
        void SetHorizontalOverflow(HorizontalWrapMode mode);
        void SetVerticalOverflow(VerticalWrapMode mode);
		const Vector<LabelLine>& GetLines() const { return m_lines; }

		virtual void FillVertices(Vector<Vector3>& vertices, Vector<Vector2>& uv, Vector<Color>& colors, Vector<unsigned short>& indices);
		virtual void FillMaterial(Ref<Material>& mat);

	protected:
		UILabel();
		void ProcessText(int& actual_width, int& actual_height);
		void ApplyAlignment(Vector3& v, const Vector2& min, const Vector2& max, const Vector2& size, int line_width, int actual_width, int actual_height);

		Ref<Font> m_font;
		FontStyle m_font_style;
		int m_font_size;
		String m_text;
		int m_line_space;
		bool m_rich;
		bool m_mono;
		TextAlignment m_alignment;
		Vector<LabelLine> m_lines;
        HorizontalWrapMode m_horizontal_overflow;
        VerticalWrapMode m_vertical_overflow;
	};
}
