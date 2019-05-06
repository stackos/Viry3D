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

#include "SwitchButton.h"

namespace Viry3D
{
    SwitchButton::SwitchButton():
        m_on(true)
    {
        this->SetOnClick([this]() {
            m_on = !m_on;
            
            if (m_on)
            {
                this->SetTexture(m_on_texture);
            }
            else
            {
                this->SetTexture(m_off_texture);
            }

            if (m_on_state_change)
            {
                m_on_state_change(m_on);
            }
        });
    }

    SwitchButton::~SwitchButton()
    {
        
    }

    void SwitchButton::SetOnTexture(const Ref<Texture>& texture)
    {
        m_on_texture = texture;

        if (m_on)
        {
            this->SetTexture(m_on_texture);
        }
    }

    void SwitchButton::SetOffTexture(const Ref<Texture>& texture)
    {
        m_off_texture = texture;

        if (!m_on)
        {
            this->SetTexture(m_off_texture);
        }
    }

    void SwitchButton::SetSwitchState(bool on)
    {
        if (m_on != on)
        {
            m_on = on;

            if (m_on)
            {
                this->SetTexture(m_on_texture);
            }
            else
            {
                this->SetTexture(m_off_texture);
            }

            if (m_on_state_change)
            {
                m_on_state_change(m_on);
            }
        }
    }

    void SwitchButton::SetOnSwitchStateChange(OnSwitchStateChange func)
    {
        m_on_state_change = func;
    }
}
