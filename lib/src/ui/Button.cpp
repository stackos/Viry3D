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

#include "Button.h"
#include "Label.h"
#include "Font.h"
#include "Application.h"

namespace Viry3D
{
    Button::Button():
        m_touch_down(false)
    {
        this->SetOnTouchDownInside([this](const Vector2i& pos) {
            this->m_touch_down = true;
            return true;
        });
        this->SetOnTouchUpInside([this](const Vector2i& pos) {
            if (this->m_touch_down)
            {
                this->OnClick();
                this->m_touch_down = false;
            }
            return true;
        });
        this->SetOnTouchUpOutside([this](const Vector2i& pos) {
            this->m_touch_down = false;
            return false;
        });
    }
    
    Button::~Button()
    {
    
    }

    const Ref<Label>& Button::GetLabel()
    {
        if (!m_label)
        {
            m_label = RefMake<Label>();
            m_label->SetSize(Vector2i(VIEW_SIZE_FILL_PARENT, VIEW_SIZE_FILL_PARENT));
            m_label->SetFont(Font::GetFont(FontType::Consola));
            m_label->SetColor(Color(0, 0, 0, 1));

            this->AddSubview(m_label);
        }

        return m_label;
    }

    void Button::OnClick() const
    {
        if (m_on_click)
        {
            Application::Instance()->PostAction(m_on_click);
        }
    }
}
