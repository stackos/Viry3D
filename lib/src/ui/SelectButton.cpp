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

#include "SelectButton.h"
#include "Label.h"

namespace Viry3D
{
    SelectButton::SelectButton():
        m_select(-1)
    {
        this->SetOnClick([this]() {
            
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
