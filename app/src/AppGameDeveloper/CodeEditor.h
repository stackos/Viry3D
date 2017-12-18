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

#pragma once

#include "Component.h"

namespace Viry3D
{
	class Camera;
	class RenderTexture;
	class UICanvasRenderer;
	class UILabel;
	class Font;

	struct CodeLine
	{
		String text;
		int line;
		Ref<UICanvasRenderer> canvas;
		Ref<UILabel> label;
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

	protected:
		CodeEditor();

	protected:
		int m_render_depth;
		int m_target_screen_width;
		int m_target_screen_height;
		Ref<Camera> m_camera;
		Ref<UICanvasRenderer> m_canvas;
		String m_source_code;
		Vector<CodeLine> m_lines;
		Ref<Font> m_font;
		int m_font_size;
		int m_line_space;
	};
}
