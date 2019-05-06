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

#include "SelectButton.h"
#include "Label.h"
#include "CanvasRenderer.h"
#include "graphics/Display.h"
#include "graphics/Texture.h"

namespace Viry3D
{
    SelectButton::SelectButton():
        m_select(-1)
    {
        this->SetOnClick([this]() {
            if (!m_select_view)
            {
                Ref<View> view = RefMake<View>();
                view->SetAlignment(ViewAlignment::HCenter | ViewAlignment::Bottom);
                view->SetPivot(Vector2(0.5f, 0));
                view->SetSize(Vector2i(VIEW_SIZE_FILL_PARENT, this->GetSize().y * m_names.Size()));
                view->SetOffset(Vector2i(0, 0));

                for (int i = 0; i < m_names.Size(); ++i)
                {
                    auto button = RefMake<Button>();
                    button->SetAlignment(ViewAlignment::HCenter | ViewAlignment::Top);
                    button->SetPivot(Vector2(0.5f, 0));
                    button->SetSize(Vector2i(VIEW_SIZE_FILL_PARENT, this->GetSize().y));
                    button->SetOffset(Vector2i(0, this->GetSize().y * i));
                    button->SetTexture(this->GetTexture());
                    button->SetColor(this->GetColor());
                    button->GetLabel()->SetColor(this->GetLabel()->GetColor());
                    button->GetLabel()->SetText(m_names[i]);
                    button->SetOnClick([=]() {
                        this->RemoveSubview(m_select_view);
                        this->SetSelect(i);
                    });

                    view->AddSubview(button);
                }

                m_select_view = view;
            }

            if (m_select_view->GetParentView() == nullptr)
            {
                this->AddSubview(m_select_view);
            }
            else
            {
                this->RemoveSubview(m_select_view);
            }
        });
    }

    SelectButton::~SelectButton()
    {
        
    }

    void SelectButton::SetSelectNames(const Vector<String>& names)
    {
        m_names = names;
    }

    void SelectButton::SetSelect(int index)
    {
        if (m_select != index)
        {
            m_select = index;

            if (m_select >= 0 && m_select < m_names.Size())
            {
                this->GetLabel()->SetText(m_names[m_select]);
            }
            else
            {
                this->GetLabel()->SetText("");
            }

            if (m_on_select_change)
            {
                m_on_select_change(m_select);
            }
        }
    }

    void SelectButton::SetOnSelectChange(OnSelectChange func)
    {
        m_on_select_change = func;
    }
}
