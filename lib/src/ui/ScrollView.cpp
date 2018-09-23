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

#include "ScrollView.h"

namespace Viry3D
{
    ScrollView::ScrollView():
        m_scroll_threhold(10)
    {
        m_content_view = RefMake<View>();
        this->AddSubview(m_content_view);

        m_content_view->SetSize(Vector2i(0, 0));
        m_content_view->SetAlignment(ViewAlignment::HCenter | ViewAlignment::Top);
        m_content_view->SetPivot(Vector2(0.5f, 0));
        m_content_view->SetOffset(Vector2i(0, 0));

        this->SetOnTouchDownInside([this](const Vector2i& pos) {
            return false;
        });
        this->SetOnTouchDrag([this](const Vector2i& pos) {
            return false;
        });
        this->SetOnTouchUpInside([this](const Vector2i& pos) {
            return this->OnTouchUp(pos);
        });
        this->SetOnTouchUpOutside([this](const Vector2i& pos) {
            return this->OnTouchUp(pos);
        });
    }

    ScrollView::~ScrollView()
    {
    
    }

    void ScrollView::SetContentViewSize(const Vector2i& size)
    {
        m_content_view->SetSize(size);
    }

    void ScrollView::SetScrollThrehold(float threhold)
    {
        m_scroll_threhold = threhold;
    }

    bool ScrollView::OnTouchUp(const Vector2i& pos)
    {
        return false;
    }
}
