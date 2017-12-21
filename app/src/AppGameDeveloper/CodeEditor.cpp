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

#include "CodeEditor.h"
#include "GameObject.h"
#include "Resource.h"
#include "graphics/Camera.h"
#include "graphics/RenderTexture.h"
#include "ui/UICanvasRenderer.h"
#include "ui/UILabel.h"
#include "ui/Font.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(CodeEditor);

	void CodeEditor::DeepCopy(const Ref<Object>& source)
	{
		Component::DeepCopy(source);
	}

	CodeEditor::CodeEditor():
		m_target_screen_width(0),
		m_target_screen_height(0),
		m_render_depth(0),
		m_font_size(18),
		m_line_space(2)
	{
	}

	void CodeEditor::SetRenderDepth(int depth)
	{
		m_render_depth = depth;
	}

	void CodeEditor::SetTargetScreenSize(int width, int height)
	{
		m_target_screen_width = width;
		m_target_screen_height = height;
	}

	void CodeEditor::CreateCamera()
	{
		if (m_target_screen_width > 0 &&
			m_target_screen_height > 0)
		{
			int layer = this->GetGameObject()->GetLayer();

			auto camera = GameObject::Create("Camera")->AddComponent<Camera>();
			camera->GetGameObject()->SetLayer(layer);
			camera->GetTransform()->SetParent(this->GetTransform());
			camera->SetCullingMask(1 << layer);
			camera->SetDepth(m_render_depth);
			camera->SetClearColor(Color(30, 30, 30, 255) / 255.0f);

			auto render_target = RefMake<FrameBuffer>();
			render_target->color_texture = RenderTexture::Create(m_target_screen_width, m_target_screen_height, RenderTextureFormat::RGBA32, DepthBuffer::Depth_0, FilterMode::Bilinear);
			render_target->depth_texture = RenderTexture::Create(m_target_screen_width, m_target_screen_height, RenderTextureFormat::Depth, DepthBuffer::Depth_24, FilterMode::Bilinear);
			camera->SetFrameBuffer(render_target);

			camera->SetOrthographic(true);
			camera->SetOrthographicSize(camera->GetTargetHeight() / 2.0f);
			camera->SetClipNear(-1);
			camera->SetClipFar(1);

			m_camera = camera;

			auto canvas = GameObject::Create("Canvas")->AddComponent<UICanvasRenderer>();
			canvas->GetGameObject()->SetLayer(layer);
			canvas->GetTransform()->SetParent(this->GetTransform());
			canvas->SetAnchors(Vector2(0, 0), Vector2(0, 0));
			canvas->SetOffsets(Vector2(0, 0), Vector2((float) m_target_screen_width, (float) m_target_screen_height));
			canvas->SetPivot(Vector2(0.5f, 0.5f));
			canvas->SetSize(Vector2((float) m_target_screen_width, (float) m_target_screen_height));
			canvas->OnAnchor();
			canvas->SetSortingOrder(10000);
			canvas->GetTransform()->SetPosition(Vector3::Zero());
			canvas->GetTransform()->SetScale(Vector3::One());
			canvas->SetCamera(camera);

			m_canvas = canvas;
		}
	}

	Ref<RenderTexture> CodeEditor::GetTargetRenderTexture() const
	{
		Ref<RenderTexture> texture;

		if (m_camera)
		{
			texture = m_camera->GetFrameBuffer()->color_texture;
		}

		return texture;
	}

	void CodeEditor::SetFontSize(int size)
	{
		m_font_size = size;
	}

	void CodeEditor::SetLineSpace(int space)
	{
		m_line_space = space;
	}

	void CodeEditor::Clear()
	{
		for (auto& i : m_lines)
		{
			GameObject::Destroy(i->canvas->GetGameObject());
		}
		m_lines.Clear();
	}

	void CodeEditor::LoadSource(const String& source)
	{
		this->Clear();

		if (!m_font)
		{
			m_font = Resource::LoadFont("Assets/font/consola.ttf");
		}

		m_source_code = source;
		auto lines = m_source_code.Split("\r\n", false);

		for (int i = 0; i < lines.Size(); i++)
		{
			int layer = this->GetGameObject()->GetLayer();
			int line_height = m_font_size + m_line_space;
			const float border_x = 10;
			int line_num = i + 1;

			auto canvas = GameObject::Create("Canvas")->AddComponent<UICanvasRenderer>();
			canvas->GetGameObject()->SetLayer(layer);
			canvas->GetTransform()->SetParent(m_canvas->GetTransform());
			canvas->SetAnchors(Vector2(0, 1), Vector2(1, 1));
			canvas->SetOffsets(Vector2(border_x, -m_line_space - (float) line_height * (i + 1)), Vector2(-border_x, -m_line_space - (float) line_height * i));
			canvas->SetPivot(Vector2(0.5f, 0.5f));
			canvas->OnAnchor();
			canvas->SetSortingOrder(1000);

			String label_text = String::Format("%4d    %s", line_num, lines[i].CString());

			auto label = GameObject::Create("Label")->AddComponent<UILabel>();
			label->GetGameObject()->SetLayer(layer);
			label->GetTransform()->SetParent(canvas->GetTransform());
			label->SetAnchors(Vector2(0, 0), Vector2(1, 1));
			label->SetOffsets(Vector2(0, 0), Vector2(0, 0));
			label->SetPivot(Vector2(0.5f, 0.5f));
			label->OnAnchor();
			label->SetFont(m_font);
			label->SetFontStyle(FontStyle::Normal);
			label->SetFontSize(m_font_size);
			label->SetColor(Color(1, 1, 1, 1));
			label->SetText(label_text);
			label->SetLineSpace(1);
			label->SetRich(true);
			label->SetMono(false);
			label->SetAlignment(TextAlignment::MiddleLeft);

			auto line = RefMake<CodeLine>();
			line->text = lines[i];
			line->line = line_num;
			line->canvas = canvas;
			line->label = label;

			m_lines.AddLast(line);
		}
	}

	void CodeEditor::OnTouchDown(const Vector2& pos)
	{
		Log("OnTouchDown:%s", pos.ToString().CString());
	}

	void CodeEditor::OnTouchMove(const Vector2& pos)
	{
		Log("OnTouchMove:%s", pos.ToString().CString());
	}

	void CodeEditor::OnTouchUp(const Vector2& pos)
	{
		Log("OnTouchUp:%s", pos.ToString().CString());
	}
}
