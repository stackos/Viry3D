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

#include "CodeEditor.h"
#include "GameObject.h"
#include "Resource.h"
#include "graphics/Camera.h"
#include "graphics/RenderTexture.h"
#include "ui/UICanvasRenderer.h"
#include "ui/UILabel.h"
#include "ui/UISprite.h"
#include "ui/Font.h"
#include "time/Time.h"
#include "math/Mathf.h"

static const float CODE_CANVAS_BORDER_X = 10;
static const float CODE_TEXT_BORDER_X = 80;

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
		m_line_space(4),
		m_scroll_position(0, 0),
        m_cursor_line(NULL),
        m_cursor_char_index(-1),
        m_cursor_flash_time(-1)
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
			canvas->GetTransform()->SetLocalPosition(Vector3::Zero());
			canvas->GetTransform()->SetLocalScale(Vector3::One());
			canvas->SetCamera(camera);

			m_canvas = canvas;

            auto cursor = GameObject::Create("Cursor")->AddComponent<UISprite>();
            cursor->GetGameObject()->SetLayer(layer);
            cursor->GetTransform()->SetParent(m_canvas->GetTransform());
            cursor->SetAnchors(Vector2(0, 1), Vector2(0, 1));
            cursor->SetOffsets(Vector2(CODE_CANVAS_BORDER_X + CODE_TEXT_BORDER_X - 1, 0), Vector2(CODE_CANVAS_BORDER_X + CODE_TEXT_BORDER_X, (float) -this->GetLineHeight()));
            cursor->SetPivot(Vector2(0.5f, 0.5f));
            cursor->OnAnchor();

            m_cursor = cursor;
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

		int layer = this->GetGameObject()->GetLayer();
		int line_height = this->GetLineHeight();

		for (int i = 0; i < lines.Size(); i++)
		{
			int line_num = i + 1;

			auto canvas = GameObject::Create("Canvas")->AddComponent<UICanvasRenderer>();
			canvas->GetGameObject()->SetLayer(layer);
			canvas->GetTransform()->SetParent(m_canvas->GetTransform());
			canvas->SetAnchors(Vector2(0, 1), Vector2(1, 1));
			canvas->SetOffsets(Vector2(CODE_CANVAS_BORDER_X, - (float) line_height * (i + 1)), Vector2(-CODE_CANVAS_BORDER_X, - (float) line_height * i));
			canvas->SetPivot(Vector2(0.5f, 0.5f));
			canvas->OnAnchor();
			canvas->SetSortingOrder(1000);

            String line_num_text = String::Format("%4d", line_num);
            auto label_line_num = GameObject::Create("Label")->AddComponent<UILabel>();
            label_line_num->GetGameObject()->SetLayer(layer);
            label_line_num->GetTransform()->SetParent(canvas->GetTransform());
            label_line_num->SetAnchors(Vector2(0, 0), Vector2(1, 1));
            label_line_num->SetOffsets(Vector2(0, 0), Vector2(0, 0));
            label_line_num->SetPivot(Vector2(0.5f, 0.5f));
            label_line_num->OnAnchor();
            label_line_num->SetFont(m_font);
            label_line_num->SetFontStyle(FontStyle::Normal);
            label_line_num->SetFontSize(m_font_size);
            label_line_num->SetColor(Color(1, 1, 1, 1));
            label_line_num->SetText(line_num_text);
            label_line_num->SetLineSpace(1);
            label_line_num->SetRich(true);
            label_line_num->SetMono(false);
            label_line_num->SetAlignment(TextAlignment::MiddleLeft);

            String line_text = lines[i];
			auto label_line_text = GameObject::Create("Label")->AddComponent<UILabel>();
            label_line_text->GetGameObject()->SetLayer(layer);
            label_line_text->GetTransform()->SetParent(canvas->GetTransform());
            label_line_text->SetAnchors(Vector2(0, 0), Vector2(1, 1));
            label_line_text->SetOffsets(Vector2(CODE_TEXT_BORDER_X, 0), Vector2(0, 0));
            label_line_text->SetPivot(Vector2(0.5f, 0.5f));
            label_line_text->OnAnchor();
            label_line_text->SetFont(m_font);
            label_line_text->SetFontStyle(FontStyle::Normal);
            label_line_text->SetFontSize(m_font_size);
            label_line_text->SetColor(Color(1, 1, 1, 1));
            label_line_text->SetText(line_text);
            label_line_text->SetLineSpace(1);
            label_line_text->SetRich(true);
            label_line_text->SetMono(false);
            label_line_text->SetAlignment(TextAlignment::MiddleLeft);

			auto line = RefMake<CodeLine>();
			line->text = lines[i];
			line->line_num = line_num;
			line->canvas = canvas;
            line->label_line_num = label_line_num;
			line->label_line_text = label_line_text;

			m_lines.AddLast(line);
		}
	}

	int CodeEditor::GetLineHeight()
	{
		return m_font_size + m_line_space;
	}

	void CodeEditor::SetSrollPosition(const Vector2& pos)
	{
		m_scroll_position = pos;

		m_canvas->GetTransform()->SetLocalPosition(pos);
	}

    void CodeEditor::UpdateCursorPosition(const CodeLine* line, int char_index)
    {
        m_cursor_line = line;
        m_cursor_char_index = char_index;
        m_cursor_flash_time = -1;

        const auto& lines = line->label_line_text->GetLines();
        float x;
        float y = line->label_line_text->GetTransform()->GetLocalPosition().y;

        if (char_index >= 0)
        {
            x = lines[0].char_bounds[char_index].Min().x;
        }
        else
        {
            if (lines.Size() > 0)
            {
                const auto& bounds = lines[0].char_bounds;
                x = bounds[bounds.Size() - 1].Max().x + 2;
            }
            else
            {
                x = - (float) m_target_screen_width / 2 + CODE_CANVAS_BORDER_X + CODE_TEXT_BORDER_X;
            }
        }

        auto mat = line->canvas->GetTransform()->GetLocalToWorldMatrix();
        m_cursor->GetTransform()->SetPosition(mat.MultiplyPoint3x4(Vector3(x, y, 0)));
    }

	void CodeEditor::OnTouchDown(const Vector2& pos)
	{
        Vector3 pos_world = m_camera->ScreenToWorldPoint(pos);
        Vector3 pos_canvas = m_canvas->GetTransform()->GetWorldToLocalMatrix().MultiplyPoint3x4(pos_world);

		float offset_y = pos_canvas.y - m_target_screen_height / 2;

		CodeLine* line = NULL;

		for (auto& i : m_lines)
		{
			Vector2 offset_min = i->canvas->GetOffsetMin();
			Vector2 offset_max = i->canvas->GetOffsetMax();

			if (offset_y <= offset_max.y && offset_y > offset_min.y)
			{
				line = i.get();
				break;
			}
		}

        int char_index = -1;

		if (line)
		{
            const auto& label_lines = line->label_line_text->GetLines();
            if (label_lines.Size() > 0)
            {
                const auto& label_line = label_lines[0];

                if (pos_canvas.x < label_line.char_bounds[0].Min().x)
                {
                    char_index = 0;
                }
                else if(pos_canvas.x > label_line.char_bounds[label_line.char_bounds.Size() - 1].Max().x)
                {
                    char_index = -1;
                }
                else
                {
                    for (int i = label_line.chars.Size() - 1; i >= 0; i--)
                    {
                        const Bounds& bounds = label_line.char_bounds[i];

                        if (pos_canvas.x >= bounds.Min().x)
                        {
                            char_index = i;
                            break;
                        }
                    }
                }
            }
		}
        else if (offset_y <= m_lines.Last()->canvas->GetOffsetMin().y)
        {
            line = m_lines.Last().get();
        }

        this->UpdateCursorPosition(line, char_index);
	}

	void CodeEditor::OnTouchMove(const Vector2& pos)
	{
        
	}

	void CodeEditor::OnTouchUp(const Vector2& pos)
	{
        
	}

    void CodeEditor::UpdateCursorFlash()
    {
        if (m_cursor_flash_time < 0)
        {
            m_cursor_flash_time = Time::GetTime();
        }

        if (fmod(Time::GetTime() - m_cursor_flash_time, 1.0f) < 0.5f)
        {
            m_cursor->SetColor(Color(1, 1, 1, 1));
        }
        else
        {
            m_cursor->SetColor(Color(1, 1, 1, 0));
        }
    }

	void CodeEditor::Update()
	{
        this->UpdateCursorFlash();
	}
}
