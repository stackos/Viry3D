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

#include "Button.h"

namespace Viry3D
{
    class SelectButton: public Button
    {
    public:
        typedef std::function<void(int)> OnSelectChange;

        SelectButton();
        virtual ~SelectButton();
        void SetSelectNames(const Vector<String>& names);
        void SetSelect(int index);
        bool GetSelect() const { return m_select; }
        void SetOnSelectChange(OnSelectChange func);

    private:
        Vector<String> m_names;
        int m_select;
        OnSelectChange m_on_select_change;
        Ref<View> m_select_view;
    };
}
