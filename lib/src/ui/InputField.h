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

#pragma once

#include "Sprite.h"
#include "Action.h"

namespace Viry3D
{
    class Label;

    class InputField : public Sprite
    {
    public:
        InputField();
        virtual ~InputField();
        virtual void Update();
        void SetPlaceholderText(const String& placeholder);
        void SetPlaceholderTextColor(const Color& color);
        void SetCaretBlinkRate(float rate);
        const String& GetText() const;
        void SetText(const String& text);
        void SetOnEnter(Action on_enter) { m_on_enter = on_enter; }

    private:
        void OnGotFocus();
        void OnLostFocus();
        void OnEnter();
        void SetCaretPos(int line, int index);

    private:
        Ref<Label> m_placeholder;
        Ref<View> m_content;
        Ref<Label> m_label;
        Ref<Sprite> m_caret;
        Vector2i m_caret_pos;
        float m_caret_blink_rate;
        bool m_caret_blink_show;
        float m_caret_blink_time;
        bool m_touch_down;
        bool m_focused;
        Vector4 m_content_margin;
        Vector<char32_t> m_unicodes;
        Action m_on_enter;
    };
}
