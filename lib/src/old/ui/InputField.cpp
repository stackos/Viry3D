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

#include "InputField.h"
#include "Label.h"
#include "Font.h"
#include "time/Time.h"
#include "Input.h"

namespace Viry3D
{
    InputField::InputField():
        m_caret_pos(-1, -1),
        m_caret_blink_rate(0.5f),
        m_caret_blink_show(true),
        m_caret_blink_time(0),
        m_touch_down(false),
        m_focused(false),
        m_content_margin(10, 0, 10, 0)
    {
        m_placeholder = RefMake<Label>();
        m_placeholder->SetSize(Vector2i(VIEW_SIZE_FILL_PARENT, VIEW_SIZE_FILL_PARENT));
        m_placeholder->SetMargin(m_content_margin);
        m_placeholder->SetTextAlignment(ViewAlignment::Left | ViewAlignment::VCenter);
        m_placeholder->SetFont(Font::GetFont(FontType::PingFangSC));
        m_placeholder->SetFontSize(20);
        m_placeholder->SetColor(Color(0.8f, 0.8f, 0.8f, 1));
        m_placeholder->EnableClipRect(true);
        m_placeholder->SetText("Input");
        this->AddSubview(m_placeholder);

        m_content = RefMake<View>();
        m_content->SetSize(Vector2i(VIEW_SIZE_FILL_PARENT, VIEW_SIZE_FILL_PARENT));
        m_content->SetMargin(m_content_margin);
        m_content->EnableClipRect(true);
        this->AddSubview(m_content);

        m_label = RefMake<Label>();
        m_label->SetAlignment(ViewAlignment::Left | ViewAlignment::VCenter);
        m_label->SetPivot(Vector2(0, 0.5f));
        m_label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::VCenter);
        m_label->SetFont(Font::GetFont(FontType::PingFangSC));
        m_label->SetFontSize(20);
        m_label->SetColor(Color(0, 0, 0, 1));
        m_content->AddSubview(m_label);

        m_caret = RefMake<Sprite>();
        m_caret->SetAlignment(ViewAlignment::Left | ViewAlignment::VCenter);
        m_caret->SetSize(Vector2i(1, 20));
        m_caret->SetColor(Color(0, 0, 0, 1));
        
        this->SetOnTouchDownInside([this](const Vector2i& pos) {
            m_touch_down = true;
            return true;
        });
        this->SetOnTouchUpInside([this](const Vector2i& pos) {
            if (m_touch_down)
            {
                m_touch_down = false;
                if (!m_focused)
                {
                    m_focused = true;
                    this->OnGotFocus();
                }
            }
            return true;
        });
        this->SetOnTouchUpOutside([this](const Vector2i& pos) {
            m_touch_down = false;
            if (m_focused)
            {
                m_focused = false;
                this->OnLostFocus();
            }
            return false;
        });
    }
    
    InputField::~InputField()
    {
        
    }

    void InputField::Update()
    {
        // blink caret
        if (m_focused)
        {
            float now = Time::GetTime();
            if (now - m_caret_blink_time > m_caret_blink_rate)
            {
                if (m_caret_blink_show)
                {
                    m_caret->SetColor(Color(0, 0, 0, 0));
                }
                else
                {
                    m_caret->SetColor(Color(0, 0, 0, 1));
                }
                m_caret_blink_show = !m_caret_blink_show;
                m_caret_blink_time = now;
            }

            const Vector<unsigned short>& chars = Input::GetInputQueueCharacters();
            if (chars.Size() > 0)
            {
                Vector<char> cs;
                for (int i = 0; i < chars.Size(); ++i)
                {
                    unsigned short c = chars[i];
                    if (c <= 0xff)
                    {
                        cs.Add(c & 0xff);
                    }
                    else
                    {
                        cs.Add((c & 0xff00) >> 8);
                        cs.Add(c & 0xff);
                    }
                }

#if VR_WINDOWS
                String str = String::Gb2312ToUtf8(String(&cs[0], cs.Size()));
#else
                String str = String(&cs[0], cs.Size());
#endif

                // process \b - backspace
                Vector<String> splits = str.Split("\b");
                for (int i = 0; i < splits.Size(); ++i)
                {
                    String split = splits[i];

                    if (split.Size() > 0)
                    {
                        // process \r - enter
                        int r_index = split.IndexOf("\r");
                        if (r_index >= 0)
                        {
                            if (r_index > 0)
                            {
                                split = split.Substring(0, r_index);
                            }
                            else
                            {
                                split = "";
                            }
                        }

                        // append chars
                        if (split.Size() > 0)
                        {
                            this->SetText(m_label->GetText() + split);

                            const auto& lines = m_label->GetLines();
                            this->SetCaretPos(0, lines[0].meshes.Size() - 1);
                        }

                        if (r_index >= 0)
                        {
                            this->OnEnter();
                            break;
                        }
                    }

                    // delete char
                    if (i < splits.Size() - 1)
                    {
                        if (m_unicodes.Size() > 0)
                        {
                            m_unicodes.Remove(m_unicodes.Size() - 1);
                            if (m_unicodes.Size() > 0)
                            {
                                this->SetText(String(&m_unicodes[0], m_unicodes.Size()));

                                const auto& lines = m_label->GetLines();
                                this->SetCaretPos(0, lines[0].meshes.Size() - 1);
                            }
                            else
                            {
                                this->SetText("");
                                this->SetCaretPos(-1, -1);
                            }
                        }
                    }
                }
            }
        }
    }

    void InputField::SetPlaceholderText(const String& placeholder)
    {
        m_placeholder->SetText(placeholder);
    }

    void InputField::SetPlaceholderTextColor(const Color& color)
    {
        m_placeholder->SetColor(color);
    }

    void InputField::SetCaretBlinkRate(float rate)
    {
        m_caret_blink_rate = rate;
    }

    const String& InputField::GetText() const
    {
        return m_label->GetText();
    }

    void InputField::SetText(const String& text)
    {
        m_label->SetText(text);

        if (text.Size() > 0)
        {
            if (m_placeholder->GetParentView() == this)
            {
                this->RemoveSubview(m_placeholder);
            }
        }
        else
        {
            if (m_placeholder->GetParentView() == nullptr)
            {
                this->AddSubview(m_placeholder);
            }
        }

        m_unicodes = text.ToUnicode32();
    }

    void InputField::OnGotFocus()
    {
        this->AddSubview(m_caret);
        const auto& lines = m_label->GetLines();
        if (lines.Size() > 0)
        {
            this->SetCaretPos(0, lines[0].meshes.Size() - 1);
        }
        else
        {
            this->SetCaretPos(-1, -1);
        }
        m_caret_blink_time = Time::GetTime();
    }

    void InputField::OnLostFocus()
    {
        this->RemoveSubview(m_caret);
    }

    void InputField::OnEnter()
    {
        m_focused = false;
        this->OnLostFocus();

        if (m_on_enter)
        {
            m_on_enter();
        }
    }

    void InputField::SetCaretPos(int line, int index)
    {
        const auto& lines = m_label->GetLines();
        if (line < 0 || index < 0)
        {
            m_caret->SetOffset(Vector2i((int) m_content_margin.x, 0));
        }
        else
        {
            int x = (int) m_content_margin.x + lines[line].meshes[index].vertices[3].x;
            int max = this->GetSize().x - (int) m_content_margin.z;
            if (x > max)
            {
                m_caret->SetOffset(Vector2i(max, 0));
                m_label->SetOffset(Vector2i(max - x, 0));
            }
            else
            {
                m_caret->SetOffset(Vector2i(x, 0));
                m_label->SetOffset(Vector2i(0, 0));
            }
        }
        m_caret_pos.x = line;
        m_caret_pos.y = index;
    }
}
