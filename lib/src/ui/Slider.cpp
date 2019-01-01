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

#include "Slider.h"
#include "Sprite.h"
#include "graphics/Texture.h"
#include "Application.h"

namespace Viry3D
{
    Slider::Slider():
        m_progress(0),
        m_type(ValueType::Float),
        m_min_value(0.0f),
        m_max_value(1.0f),
        m_value(0.0f),
        m_down_pos(0, 0),
        m_slider_pos(0, 0)
    {
        auto circle = Texture::LoadTexture2DFromFile(Application::Instance()->GetDataPath() + "/texture/ui/circle.png", FilterMode::Linear, SamplerAddressMode::ClampToEdge, false, false);

        int bar_height = this->GetSize().y / 3;
        Color bar_color = Color(0.4f, 0.4f, 0.4f, 1.0f);
        Color bar_fill_color = Color(1.0f, 1.0f, 1.0f, 1.0f);

        m_bar_left = RefMake<Sprite>();
        this->AddSubview(m_bar_left);

        m_bar_left->SetAlignment(ViewAlignment::Left | ViewAlignment::VCenter);
        m_bar_left->SetPivot(Vector2(0, 0.5f));
        m_bar_left->SetSize(Vector2i(bar_height, bar_height));
        m_bar_left->SetOffset(Vector2i(0, 0));
        m_bar_left->SetTexture(circle);
        m_bar_left->SetColor(bar_fill_color);
        
        m_bar_right = RefMake<Sprite>();
        this->AddSubview(m_bar_right);

        m_bar_right->SetAlignment(ViewAlignment::Right | ViewAlignment::VCenter);
        m_bar_right->SetPivot(Vector2(1.0f, 0.5f));
        m_bar_right->SetSize(Vector2i(bar_height, bar_height));
        m_bar_right->SetOffset(Vector2i(0, 0));
        m_bar_right->SetTexture(circle);
        m_bar_right->SetColor(bar_color);

        m_bar_center_left = RefMake<Sprite>();
        this->AddSubview(m_bar_center_left);

        m_bar_center_left->SetAlignment(ViewAlignment::Left | ViewAlignment::VCenter);
        m_bar_center_left->SetPivot(Vector2(0, 0.5f));
        m_bar_center_left->SetSize(Vector2i((int) ((this->GetSize().x - bar_height) * m_progress), bar_height));
        m_bar_center_left->SetOffset(Vector2i(bar_height / 2, 0));
        m_bar_center_left->SetTexture(Texture::GetSharedWhiteTexture());
        m_bar_center_left->SetColor(bar_fill_color);

        m_bar_center_right = RefMake<Sprite>();
        this->AddSubview(m_bar_center_right);

        m_bar_center_right->SetAlignment(ViewAlignment::Right | ViewAlignment::VCenter);
        m_bar_center_right->SetPivot(Vector2(1, 0.5f));
        m_bar_center_right->SetSize(Vector2i((int) ((this->GetSize().x - bar_height) * (1.0f - m_progress)), bar_height));
        m_bar_center_right->SetOffset(Vector2i(-bar_height / 2, 0));
        m_bar_center_right->SetTexture(Texture::GetSharedWhiteTexture());
        m_bar_center_right->SetColor(bar_color);

        m_slider = RefMake<Sprite>();
        this->AddSubview(m_slider);

        m_slider->SetAlignment(ViewAlignment::Left | ViewAlignment::VCenter);
        m_slider->SetPivot(Vector2(0.5f, 0.5f));
        m_slider->SetSize(Vector2i(this->GetSize().y, this->GetSize().y));
        m_slider->SetOffset(Vector2i(bar_height / 2 + (int) ((this->GetSize().x - bar_height) * m_progress), 0));
        m_slider->SetTexture(circle);

        m_slider->SetOnTouchDownInside([this](const Vector2i& pos) {
            m_down_pos = pos;
            m_slider_pos = m_slider->GetOffset();
            return true;
        });
        m_slider->SetOnTouchDrag([this](const Vector2i& pos) {
            Vector2i offset = pos - m_down_pos;
            Vector2i new_pos = m_slider_pos + Vector2i(offset.x, 0);

            // limit
            int bar_height = this->GetSize().y / 3;
            int x_min = bar_height / 2 + (int) ((this->GetSize().x - bar_height) * 0.0f);
            int x_max = bar_height / 2 + (int) ((this->GetSize().x - bar_height) * 1.0);

            float progress = (new_pos.x - bar_height / 2) / (float) (this->GetSize().x - bar_height);

            if (new_pos.x < x_min)
            {
                new_pos.x = x_min;
                m_down_pos.x = pos.x;
                m_slider_pos = new_pos;
                progress = 0;
            }
            if (new_pos.x > x_max)
            {
                new_pos.x = x_max;
                m_down_pos.x = pos.x;
                m_slider_pos = new_pos;
                progress = 1;
            }
            
            this->SetProgress(progress);

            return true;
        });
        m_slider->SetOnTouchUpInside([this](const Vector2i& pos) {
            return this->OnTouchUp(pos);
        });
        m_slider->SetOnTouchUpOutside([this](const Vector2i& pos) {
            return this->OnTouchUp(pos);
        });
    }
    
    Slider::~Slider()
    {
    
    }

    bool Slider::OnTouchUp(const Vector2i& pos)
    {
        m_slider_pos = m_slider->GetOffset();
        return true;
    }

    void Slider::SetSize(const Vector2i& size)
    {
        View::SetSize(size);

        int bar_height = this->GetSize().y / 3;

        m_bar_left->SetSize(Vector2i(bar_height, bar_height));
        m_bar_right->SetSize(Vector2i(bar_height, bar_height));
        m_bar_center_left->SetSize(Vector2i((int) ((this->GetSize().x - bar_height) * m_progress), bar_height));
        m_bar_center_left->SetOffset(Vector2i(bar_height / 2, 0));
        m_bar_center_right->SetSize(Vector2i((int) ((this->GetSize().x - bar_height) * (1.0f - m_progress)), bar_height));
        m_bar_center_right->SetOffset(Vector2i(-bar_height / 2, 0));
        m_slider->SetSize(Vector2i(this->GetSize().y, this->GetSize().y));
        m_slider->SetOffset(Vector2i(bar_height / 2 + (int) ((this->GetSize().x - bar_height) * m_progress), 0));
    }

    void Slider::SetProgress(float progress)
    {
        m_progress = progress;

        int bar_height = this->GetSize().y / 3;

        m_bar_center_left->SetSize(Vector2i((int) ((this->GetSize().x - bar_height) * m_progress), bar_height));
        m_bar_center_right->SetSize(Vector2i((int) ((this->GetSize().x - bar_height) * (1.0f - m_progress)), bar_height));
        m_slider->SetOffset(Vector2i(bar_height / 2 + (int) ((this->GetSize().x - bar_height) * m_progress), 0));

        this->UpdateValue();
    }

    void Slider::UpdateValue()
    {
        if (m_type == ValueType::Float)
        {
            float value = m_min_value.float_value + (m_max_value.float_value - m_min_value.float_value) * m_progress;
            m_value.float_value = value;

            if (m_on_value_change)
            {
                m_on_value_change(m_value);
            }
        }
        else if (m_type == ValueType::Int)
        {
            float value = m_min_value.int_value + (m_max_value.int_value - m_min_value.int_value) * m_progress;
            int int_value = Mathf::RoundToInt(value);

            if (m_value.int_value != int_value)
            {
                m_value.int_value = int_value;

                if (m_on_value_change)
                {
                    m_on_value_change(m_value);
                }
            }
        }
    }

    void Slider::SetValueType(ValueType type, const Value& min_value, const Value& max_value)
    {
        m_type = type;
        m_min_value = min_value;
        m_max_value = max_value;

        this->UpdateValue();
    }

    void Slider::SetOnValueChange(OnValueChange func)
    {
        m_on_value_change = func;
    }
}
