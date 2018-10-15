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
        m_max_value(1.0f)
    {
        auto circle = Texture::LoadTexture2DFromFile(Application::Instance()->GetDataPath() + "/texture/ui/circle.png", FilterMode::Linear, SamplerAddressMode::ClampToEdge, false);

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
    }
    
    Slider::~Slider()
    {
    
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
    }

    void Slider::SetValueType(ValueType type)
    {
        m_type = type;
    }

    void Slider::SetMinValue(const Value& value)
    {
        m_min_value = value;
    }

    void Slider::SetMaxValue(const Value& value)
    {
        m_max_value = value;
    }

    void Slider::SetOnValueChange(OnValueChange func)
    {
        m_on_value_change = func;
    }
}
