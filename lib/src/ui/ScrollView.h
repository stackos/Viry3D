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
    class ScrollView: public View
    {
    public:
        ScrollView();
        virtual ~ScrollView();
        virtual void OnResize(int width, int height);
        void SetSize(const Vector2i& size);
        const Ref<View>& GetContentView() const { return m_content_view; }
        void SetContentViewSize(const Vector2i& size);
        void SetScrollThrehold(float threhold);
        void SetScrollOffset(const Vector2i& pos);

    private:
        bool OnTouchUp(const Vector2i& pos);

    private:
        Ref<View> m_content_view;
        Ref<View> m_scroll_view;
        float m_scroll_threhold;
        Vector2i m_down_pos;
        bool m_scroll_start_x;
        bool m_scroll_start_y;
        Vector2i m_scroll_pos;
    };
}
