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

    class Button : public Sprite
    {
    public:
        Button();
        virtual ~Button();
        const Ref<Label>& GetLabel();
        void SetOnClick(Action func) { m_on_click = func; }
        void OnClick() const;

    private:
        Ref<Label> m_label;
        bool m_touch_down;
        Action m_on_click;
    };
}
