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

namespace Viry3D
{
    InputField::InputField():
        m_touch_down(false),
        m_focused(false),
        m_multi_line(false)
    {
        m_label = RefMake<Label>();
        m_label->SetSize(Vector2i(VIEW_SIZE_FILL_PARENT, VIEW_SIZE_FILL_PARENT));
        m_label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::VCenter);
        m_label->SetFont(Font::GetFont(FontType::PingFangSC));
        m_label->SetText("Input Field");
        m_label->SetColor(Color(0, 0, 0, 1));
        this->AddSubview(m_label);

        this->SetOnTouchDownInside([this](const Vector2i& pos) {
            this->m_touch_down = true;
            return true;
        });
        this->SetOnTouchUpInside([this](const Vector2i& pos) {
            if (this->m_touch_down)
            {
                this->m_touch_down = false;
                this->m_focused = true;
            }
            return true;
        });
        this->SetOnTouchUpOutside([this](const Vector2i& pos) {
            this->m_touch_down = false;
            this->m_focused = false;
            return false;
        });
    }
    
    InputField::~InputField()
    {
        
    }
}
