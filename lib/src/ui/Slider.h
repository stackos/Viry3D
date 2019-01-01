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

#include "View.h"

namespace Viry3D
{
    class Sprite;

    class Slider : public View
    {
    public:
        enum class ValueType
        {
            Float,
            Int,
        };

        union Value
        {
            float float_value;
            int int_value;

            Value(float value):
                float_value(value)
            {
            }

            Value(int value):
                int_value(value)
            {
            }
        };
        typedef std::function<void(const Value&)> OnValueChange;

        Slider();
        virtual ~Slider();
        void SetSize(const Vector2i& size);
        void SetProgress(float progress);
        void SetValueType(ValueType type, const Value& min_value, const Value& max_value);
        void SetOnValueChange(OnValueChange func);

    private:
        bool OnTouchUp(const Vector2i& pos);
        void UpdateValue();

    private:
        Ref<Sprite> m_bar_left;
        Ref<Sprite> m_bar_right;
        Ref<Sprite> m_bar_center_left;
        Ref<Sprite> m_bar_center_right;
        Ref<Sprite> m_slider;
        float m_progress;
        ValueType m_type;
        Value m_min_value;
        Value m_max_value;
        Value m_value;
        OnValueChange m_on_value_change;
        Vector2i m_down_pos;
        Vector2i m_slider_pos;
    };
}
