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

#include "Component.h"
#include "math/Vector2.h"

namespace Viry3D
{
	class Camera;
	class RenderTexture;
	class UICanvasRenderer;
	class UILabel;
    class UISprite;
	class Font;
	class UIPointerEvent;

	struct CodeLine
	{
		String text;
		int line_num;
		Ref<UICanvasRenderer> canvas;
        Ref<UILabel> label_line_num;
		Ref<UILabel> label_line_text;
	};

	class CodeEditor: public Component
	{
		DECLARE_COM_CLASS(CodeEditor, Component);

	public:
		void SetRenderDepth(int depth);
		void SetTargetScreenSize(int width, int height);
		void CreateCamera();
		Ref<RenderTexture> GetTargetRenderTexture() const;
		void SetFontSize(int size);
		void SetLineSpace(int space);
		void LoadSource(const String& source);

		void OnTouchDown(const Vector2& pos);
		void OnTouchMove(const Vector2& pos);
		void OnTouchUp(const Vector2& pos);

	protected:
		CodeEditor();
		virtual void Update();
		void Clear();
		int GetLineHeight();
		void SetSrollPosition(const Vector2& pos);
        void UpdateCursorPosition(const CodeLine* line, int char_index);
        void UpdateCursorFlash();

	protected:
		int m_render_depth;
		int m_target_screen_width;
		int m_target_screen_height;
		Ref<Camera> m_camera;
		Ref<UICanvasRenderer> m_canvas;
        Ref<UISprite> m_cursor;
		String m_source_code;
		List<Ref<CodeLine>> m_lines;
		Ref<Font> m_font;
		int m_font_size;
		int m_line_space;
		Vector2 m_scroll_position;
        const CodeLine* m_cursor_line;
        int m_cursor_char_index;
        float m_cursor_flash_time;
	};
}
