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
#include "LuaRunner.h"
#include "GameObject.h"
#include "Resource.h"
#include "Input.h"
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

    static const String g_token_colors[] = {
        "<color=#57A64AFF>", // Comment
        "<color=#FFFFFFFF>", // Whitespace
        "<color=#FF8000FF>", // Function
        "<color=#B4B4B4FF>", // Operator
        "<color=#D69D85FF>", // String
        "<color=#B5CEA8FF>", // Number
        "<color=#569CD6FF>", // Keyword
        "<color=#BDB76BFF>", // Identifier
        "<color=#FF0000FF>", // Default
        "<color=#FFD700FF>", // Library
        "<color=#BD63C5FF>", // Constant
        "<color=#FFFFFFFF>", // Label
    };
    static const String g_token_color_end = "</color>";

    String CodeEditor::ApplySyntaxColors(const String& source)
    {
        auto tokens = LuaRunner::Lex(source);

        String code_colored;
        int from = 0;
        for (int i = 0; i < tokens.Size(); i++)
        {
            int to = tokens[i].pos;
            String range = source.Substring(from, to - from - 1);
            auto lines = range.Split("\n", false);
            for (int j = 0; j < lines.Size(); j++)
            {
                if (lines[j].Size() > 0)
                {
                    code_colored += g_token_colors[(int) tokens[i].type] + lines[j] + g_token_color_end;
                }

                if (j < lines.Size() - 1)
                {
                    code_colored += "\n";
                }
            }

            from = to - 1;
        }

        return code_colored;
    }

    String CodeEditor::ApplyLineSyntaxColors(const String& line, bool& in_comment_block, bool& is_comment_block)
    {
        String code_colored;

        const String& token_color_comment = g_token_colors[(int) LuaTokenType::Comment];

        if (in_comment_block)
        {
            int block_end = line.IndexOf("]]");
            if (block_end >= 0)
            {
                String left = line.Substring(0, block_end + 2);
                code_colored = token_color_comment + left + g_token_color_end;

                in_comment_block = false;
                String right = line.Substring(block_end + 2);
                code_colored += CodeEditor::ApplyLineSyntaxColors(right, in_comment_block, is_comment_block);
            }
            else
            {
                code_colored = token_color_comment + line + g_token_color_end;
            }
        }
        else
        {
            int block_begin = line.IndexOf("--[[");
            if (block_begin >= 0)
            {
                String left = line.Substring(0, block_begin);
                code_colored = CodeEditor::ApplySyntaxColors(left);
                code_colored += token_color_comment + "--[[" + g_token_color_end;

                in_comment_block = true;
                is_comment_block = true;
                String right = line.Substring(block_begin + 4);
                code_colored += CodeEditor::ApplyLineSyntaxColors(right, in_comment_block, is_comment_block);
            }
            else
            {
                code_colored = CodeEditor::ApplySyntaxColors(line);
            }
        }

        return code_colored;
    }

    void CodeEditor::LoadSource(const String& source)
    {
        this->Clear();

        if (!m_font)
        {
            m_font = Resource::LoadFont("Assets/font/consola.ttf");
        }

        String source_code = source.Replace("\r\n", "\n").Replace("\r", "\n");
        auto lines = source_code.Split("\n", false);
        bool in_comment_block = false;
        
        for (int i = 0; i < lines.Size(); i++)
        {
            int line_num = i + 1;
            auto line = this->NewLine(line_num, lines[i], in_comment_block);
            m_lines.AddLast(line);
        }

        this->UpdateCursorPosition(m_lines.begin(), (*m_lines.begin())->text.Size() > 0 ? 0 : -1);
    }

    Ref<CodeLine> CodeEditor::NewLine(int line_num, const String& line_text, bool& in_comment_block)
    {
        int layer = this->GetGameObject()->GetLayer();
        int line_height = this->GetLineHeight();

        auto canvas = GameObject::Create("Canvas")->AddComponent<UICanvasRenderer>();
        canvas->GetGameObject()->SetLayer(layer);
        canvas->GetTransform()->SetParent(m_canvas->GetTransform());
        canvas->SetAnchors(Vector2(0, 1), Vector2(1, 1));
        canvas->SetOffsets(Vector2(CODE_CANVAS_BORDER_X, -(float) line_height * line_num), Vector2(-CODE_CANVAS_BORDER_X, -(float) line_height * (line_num - 1)));
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
        label_line_num->SetAlignment(TextAlignment::UpperLeft);
        label_line_num->SetColor(Color(43, 145, 174, 255) / 255.0f);

        bool is_comment_block = false;
        if (in_comment_block)
        {
            is_comment_block = true;
        }
        String line_text_colored = CodeEditor::ApplyLineSyntaxColors(line_text, in_comment_block, is_comment_block);

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
        label_line_text->SetText(line_text_colored);
        label_line_text->SetLineSpace(1);
        label_line_text->SetRich(true);
        label_line_text->SetMono(false);
        label_line_text->SetAlignment(TextAlignment::UpperLeft);
        label_line_text->SetHorizontalOverflow(HorizontalWrapMode::Overflow);

        canvas->UpdateViews();

        auto line = RefMake<CodeLine>();
        line->text = line_text;
        line->line_num = line_num;
        line->canvas = canvas;
        line->label_line_num = label_line_num;
        line->label_line_text = label_line_text;
        line->is_comment_block = is_comment_block;

        return line;
    }

    int CodeEditor::GetLineHeight()
    {
        return m_font_size + m_line_space;
    }

    void CodeEditor::SetSrollPosition(const Vector2& pos)
    {
        m_scroll_position = pos;

        if (m_canvas)
        {
            m_canvas->GetTransform()->SetLocalPosition(pos);
        }
    }

    void CodeEditor::UpdateCursorPosition(const FastList<Ref<CodeLine>>::Iterator& line, int char_index)
    {
        const auto& lines = (*line)->label_line_text->GetLines();
        float x;
        float y = (*line)->label_line_text->GetTransform()->GetLocalPosition().y;

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
                x = -(float) m_target_screen_width / 2 + CODE_CANVAS_BORDER_X + CODE_TEXT_BORDER_X;
            }
        }

        auto mat = (*line)->canvas->GetTransform()->GetLocalToWorldMatrix();
        m_cursor->GetTransform()->SetPosition(mat.MultiplyPoint3x4(Vector3(x, y, 0)));

        m_cursor_line = line;
        m_cursor_char_index = char_index;
        m_cursor_flash_time = -1;

        // scroll canvas if cursor outside screen
        float line_top = (*m_cursor_line)->canvas->GetOffsetMax().y + m_scroll_position.y;
        float line_bottom = (*m_cursor_line)->canvas->GetOffsetMin().y + m_scroll_position.y;
        if (line_bottom < -m_target_screen_height)
        {
            this->SetSrollPosition(m_scroll_position + Vector2(0, -m_target_screen_height - line_bottom));
        }
        else if(line_top > 0)
        {
            this->SetSrollPosition(m_scroll_position - Vector2(0, line_top - 0));
        }
    }

    void CodeEditor::OnTouchDown(const Vector2& pos)
    {
        Vector3 pos_world = m_camera->ScreenToWorldPoint(pos);
        Vector3 pos_canvas = m_canvas->GetTransform()->GetWorldToLocalMatrix().MultiplyPoint3x4(pos_world);

        float offset_y = pos_canvas.y - m_target_screen_height / 2;

        FastList<Ref<CodeLine>>::Iterator line = m_lines.end();

        for (auto i = m_lines.begin(); i != m_lines.end(); ++i)
        {
            Vector2 offset_min = (*i)->canvas->GetOffsetMin();
            Vector2 offset_max = (*i)->canvas->GetOffsetMax();

            if (offset_y <= offset_max.y && offset_y > offset_min.y)
            {
                line = i;
                break;
            }
        }

        int char_index = -1;

        if (line != m_lines.end())
        {
            const auto& label_lines = (*line)->label_line_text->GetLines();
            if (label_lines.Size() > 0)
            {
                const auto& label_line = label_lines[0];

                if (pos_canvas.x < label_line.char_bounds[0].Min().x)
                {
                    char_index = 0;
                }
                else if (pos_canvas.x > label_line.char_bounds[label_line.char_bounds.Size() - 1].Max().x)
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
        else if (offset_y <= (*(m_lines.end().Prev()))->canvas->GetOffsetMin().y)
        {
            line = m_lines.end().Prev();
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

    void CodeEditor::ApplyLineSyntaxColors(const FastList<Ref<CodeLine>>::Iterator& line, bool& in_comment_block)
    {
        FastList<Ref<CodeLine>>::Iterator first_not_comment_line = line;
        if ((*line)->is_comment_block)
        {
            while (first_not_comment_line != m_lines.begin())
            {
                if (!(*first_not_comment_line)->is_comment_block)
                {
                    break;
                }
                else
                {
                    --first_not_comment_line;
                }
            }
        }

        while (first_not_comment_line != line)
        {
            const String& line_text = (*first_not_comment_line)->text;
            bool is_comment_block = false;
            String line_text_colored = CodeEditor::ApplyLineSyntaxColors(line_text, in_comment_block, is_comment_block);
            ++first_not_comment_line;
        }

        bool is_comment_block = false;
        if (in_comment_block)
        {
            is_comment_block = true;
        }

        String left_colored = CodeEditor::ApplyLineSyntaxColors((*line)->text, in_comment_block, is_comment_block);
        (*line)->label_line_text->SetText(left_colored);
        (*line)->is_comment_block = is_comment_block;
    }

    void CodeEditor::InsertLine()
    {
        String left;
        String right;

        if (m_cursor_char_index >= 0)
        {
            left = (*m_cursor_line)->text.Substring(0, m_cursor_char_index);
            right = (*m_cursor_line)->text.Substring(m_cursor_char_index);
        }
        else
        {
            left = (*m_cursor_line)->text;
            right = "";
        }

        int space_count = 0;
        for (int i = 0; i < left.Size(); ++i)
        {
            if (left[i] == ' ')
            {
                ++space_count;
            }
            else
            {
                break;
            }
        }

        if (space_count > 0)
        {
            Vector<char> spaces(space_count, ' ');
            right = String(&spaces[0], space_count) + right;
        }

        bool in_comment_block = false;
        
        // left line
        {
            (*m_cursor_line)->text = left;
            this->ApplyLineSyntaxColors(m_cursor_line, in_comment_block);
        }
        
        // right new line
        {
            auto line = this->NewLine((*m_cursor_line)->line_num + 1, right, in_comment_block);
            auto new_line = m_lines.AddAfter(m_cursor_line, line);
            
            int new_index;
            if ((*new_line)->text.Size() > 0)
            {
                if ((*new_line)->text.Size() > space_count)
                {
                    new_index = space_count;
                }
                else
                {
                    new_index = -1;
                }
            }
            else
            {
                new_index = -1;
            }

            this->UpdateCursorPosition(new_line, new_index);
        }

        // update below lines
        {
            auto line = m_cursor_line.Next();
            while (line != m_lines.end())
            {
                int line_num = (*line)->line_num + 1;
                int line_height = this->GetLineHeight();

                (*line)->line_num = line_num;
                (*line)->label_line_num->SetText(String::Format("%4d", line_num));
                (*line)->canvas->SetOffsets(Vector2(CODE_CANVAS_BORDER_X, -(float) line_height * line_num), Vector2(-CODE_CANVAS_BORDER_X, -(float) line_height * (line_num - 1)));
                (*line)->canvas->OnAnchor();

                ++line;
            }
        }
    }

    void CodeEditor::RemoveChar()
    {
        const String& line_text = (*m_cursor_line)->text;

        if (m_cursor_char_index > 0 ||
            (m_cursor_char_index == -1 && line_text.Size() > 0))
        {
            if (m_cursor_char_index > 0)
            {
                String text = line_text.Substring(0, m_cursor_char_index - 1) + line_text.Substring(m_cursor_char_index);
                (*m_cursor_line)->text = text;

                bool in_comment_block = false;
                this->ApplyLineSyntaxColors(m_cursor_line, in_comment_block);
                (*m_cursor_line)->canvas->UpdateViews();

                this->UpdateCursorPosition(m_cursor_line, m_cursor_char_index - 1);
            }
            else
            {
                String text = line_text.Substring(0, line_text.Size() - 1);
                (*m_cursor_line)->text = text;

                bool in_comment_block = false;
                this->ApplyLineSyntaxColors(m_cursor_line, in_comment_block);
                (*m_cursor_line)->canvas->UpdateViews();

                this->UpdateCursorPosition(m_cursor_line, -1);
            }
        }
        else if (m_cursor_char_index == 0 ||
            (m_cursor_char_index == -1 && line_text.Size() == 0))
        {
            if (m_cursor_line != m_lines.begin())
            {
                auto prev = m_cursor_line.Prev();

                if (line_text.Size() == 0)
                {
                    this->UpdateCursorPosition(prev, -1);
                }
                else
                {
                    int prev_size = (*prev)->text.Size();
                    String text = (*prev)->text + line_text;
                    (*prev)->text = text;

                    bool in_comment_block = false;
                    this->ApplyLineSyntaxColors(prev, in_comment_block);
                    (*prev)->canvas->UpdateViews();

                    this->UpdateCursorPosition(prev, prev_size);
                }

                // update below lines
                {
                    auto line = m_cursor_line.Next();
                    GameObject::Destroy((*line)->canvas->GetGameObject());
                    line = m_lines.Remove(line);

                    while (line != m_lines.end())
                    {
                        int line_num = (*line)->line_num - 1;
                        int line_height = this->GetLineHeight();

                        (*line)->line_num = line_num;
                        (*line)->label_line_num->SetText(String::Format("%4d", line_num));
                        (*line)->canvas->SetOffsets(Vector2(CODE_CANVAS_BORDER_X, -(float) line_height * line_num), Vector2(-CODE_CANVAS_BORDER_X, -(float) line_height * (line_num - 1)));
                        (*line)->canvas->OnAnchor();

                        ++line;
                    }
                }
            }
        }
    }

    void CodeEditor::InsertString(const String& str)
    {
        const String& line_text = (*m_cursor_line)->text;

        String text;

        if (m_cursor_char_index >= 0)
        {
            text = line_text.Substring(0, m_cursor_char_index) + str + line_text.Substring(m_cursor_char_index);
        }
        else
        {
            text = line_text + str;
        }

        (*m_cursor_line)->text = text;

        bool in_comment_block = false;
        this->ApplyLineSyntaxColors(m_cursor_line, in_comment_block);
        (*m_cursor_line)->canvas->UpdateViews();

        if (m_cursor_char_index >= 0)
        {
            this->UpdateCursorPosition(m_cursor_line, m_cursor_char_index + str.Size());
        }
        else
        {
            this->UpdateCursorPosition(m_cursor_line, -1);
        }
    }

    void CodeEditor::Update()
    {
        this->UpdateCursorFlash();
        m_input_handler.HandleKeyEvents(this);
    }
}
