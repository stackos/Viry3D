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

#pragma once

#include "CameraClearFlags.h"
#include "Color.h"
#include "math/Rect.h"

namespace Viry3D
{
    class Camera
    {
    public:
        Camera();
        ~Camera();

        CameraClearFlags GetClearFlags() const { return m_clear_flags; }
        void SetClearFlags(CameraClearFlags flags);
        const Color& GetClearColor() const { return m_clear_color; }
        void SetClearColor(const Color& color);
        const Rect& GetViewportRect() const { return m_viewport_rect; }
        void SetViewportRect(const Rect& rect);
        void Update();

    private:
        bool m_render_pass_dirty;
        CameraClearFlags m_clear_flags;
        Color m_clear_color;
        Rect m_viewport_rect;
    };
}
