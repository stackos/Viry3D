#pragma once

#include "UIView.h"
#include "Font.h"

namespace Viry3D
{
	struct FontStyle
	{
		enum Enum
		{
			Normal,
			Bold,
			Italic,
			BoldAndItalic
		};
	};

	struct TextAlignment
	{
		enum Enum
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
	};

	struct LabelLine
	{
		int width;
		int height;
		Vector<Vector2> vertices;
		Vector<Vector2> uv;
		Vector<Color> colors;
		Vector<unsigned short> indices;

		void Clear()
		{
			vertices.Clear();
			uv.Clear();
			colors.Clear();
			indices.Clear();
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
	class UILabel : public UIView
	{
		DECLARE_COM_CLASS(UILabel, UIView);
	public:
		void SetFont(const Ref<Font>& font);
		void SetFontStyle(FontStyle::Enum style);
		void SetFontSize(int size);
		void SetText(const String& text);
		void SetLineSpace(int space);
		void SetRich(bool rich);
		void SetAlignment(TextAlignment::Enum alignment);

		virtual void FillVertices(Vector<Vector3>& vertices, Vector<Vector2>& uv, Vector<Color>& colors, Vector<unsigned short>& indices);
		virtual void FillMaterial(Ref<Material>& mat);

	protected:
		UILabel();
		Vector<LabelLine> ProcessText(int& actual_width, int& actual_height);

		Ref<Font> m_font;
		FontStyle::Enum m_font_style;
		int m_font_size;
		String m_text;
		int m_line_space;
		bool m_rich;
		TextAlignment::Enum m_alignment;
	};
}
