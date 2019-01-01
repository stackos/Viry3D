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

#include "ScrollView.h"

namespace Viry3D
{
    ScrollView::ScrollView():
        m_scroll_threhold(10),
        m_down_pos(0, 0),
        m_scroll_start_x(false),
        m_scroll_start_y(false),
        m_scroll_pos(0, 0)
    {
        m_content_view = RefMake<View>();
        this->AddSubview(m_content_view);

        m_content_view->SetAlignment(ViewAlignment::HCenter | ViewAlignment::Top);
        m_content_view->SetPivot(Vector2(0.5f, 0));
        m_content_view->SetSize(Vector2i(0, 0));
        m_content_view->SetOffset(Vector2i(0, 0));

        m_scroll_view = RefMake<View>();
        this->AddSubview(m_scroll_view);

        m_scroll_view->SetAlignment(ViewAlignment::HCenter | ViewAlignment::VCenter);
        m_scroll_view->SetPivot(Vector2(0.5f, 0.5f));
        m_scroll_view->SetSize(Vector2i(0, 0));
        m_scroll_view->SetOffset(Vector2i(0, 0));

        m_scroll_view->SetOnTouchDownInside([this](const Vector2i& pos) {
            m_down_pos = pos;
            m_scroll_pos = m_content_view->GetOffset();
            return false;
        });
        m_scroll_view->SetOnTouchDrag([this](const Vector2i& pos) {
            Vector2i offset = pos - m_down_pos;
            Vector2i scroll_size = this->GetCalculateddSize();
            Vector2i content_size = m_content_view->GetCalculateddSize();

            if (content_size.x > scroll_size.x)
            {
                if (!m_scroll_start_x)
                {
                    if (Mathf::Abs(offset.x) > m_scroll_threhold)
                    {
                        m_scroll_start_x = true;
                    }
                }
            }
            else if (content_size.y > scroll_size.y)
            {
                if (!m_scroll_start_y)
                {
                    if (Mathf::Abs(offset.y) > m_scroll_threhold)
                    {
                        m_scroll_start_y = true;
                    }
                }
            }

            if (!m_scroll_start_x)
            {
                offset.x = 0;
            }

            if (m_scroll_start_y)
            {
                offset.y = -offset.y;
            }
            else
            {
                offset.y = 0;
            }

            Vector2i new_pos = m_scroll_pos + offset;
            
            // limit
            if (content_size.x > scroll_size.x)
            {
                int max_scroll = content_size.x - scroll_size.x;
                if (new_pos.x < -max_scroll)
                {
                    new_pos.x = -max_scroll;
                    m_down_pos.x = pos.x;
                    m_scroll_pos = new_pos;
                }
                if (new_pos.x > 0)
                {
                    new_pos.x = 0;
                    m_down_pos.x = pos.x;
                    m_scroll_pos = new_pos;
                }
            }
            else if (content_size.y > scroll_size.y)
            {
                int max_scroll = content_size.y - scroll_size.y;
                if (new_pos.y < -max_scroll)
                {
                    new_pos.y = -max_scroll;
                    m_down_pos.y = pos.y;
                    m_scroll_pos = new_pos;
                }
                if (new_pos.y > 0)
                {
                    new_pos.y = 0;
                    m_down_pos.y = pos.y;
                    m_scroll_pos = new_pos;
                }
            }

            m_content_view->SetOffset(new_pos);

            bool block_event = m_scroll_start_x || m_scroll_start_y;
            return block_event;
        });
        m_scroll_view->SetOnTouchUpInside([this](const Vector2i& pos) {
            return this->OnTouchUp(pos);
        });
        m_scroll_view->SetOnTouchUpOutside([this](const Vector2i& pos) {
            return this->OnTouchUp(pos);
        });
    }

    ScrollView::~ScrollView()
    {
    
    }

    void ScrollView::OnResize(int width, int height)
    {
        View::OnResize(width, height);

        this->SetScrollOffset(Vector2i(0, 0));
    }

    void ScrollView::SetSize(const Vector2i& size)
    {
        View::SetSize(size);

        m_scroll_view->SetSize(size);
    }

    void ScrollView::SetContentViewSize(const Vector2i& size)
    {
        m_content_view->SetSize(size);
    }

    void ScrollView::SetScrollThrehold(float threhold)
    {
        m_scroll_threhold = threhold;
    }

    void ScrollView::SetScrollOffset(const Vector2i& pos)
    {
        m_content_view->SetOffset(pos);
    }

    bool ScrollView::OnTouchUp(const Vector2i& pos)
    {
        bool block_event = m_scroll_start_x || m_scroll_start_y;
        m_scroll_start_x = false;
        m_scroll_start_y = false;
        m_scroll_pos = m_content_view->GetOffset();
        return block_event;
    }
}
