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

#include "Camera.h"

namespace Viry3D
{
    Camera::Camera():
        m_render_pass_dirty(true),
        m_clear_flags(CameraClearFlags::ColorAndDepth),
        m_clear_color(0, 0, 0, 1),
        m_viewport_rect(0, 0, 1, 1)
    {
    
    }

    Camera::~Camera()
    {
    
    }

    void Camera::SetClearFlags(CameraClearFlags flags)
    {
        m_clear_flags = flags;
        m_render_pass_dirty = true;
    }

    void Camera::SetClearColor(const Color& color)
    {
        m_clear_color = color;
    }

    void Camera::SetViewportRect(const Rect& rect)
    {
        m_viewport_rect = rect;
    }

    void Camera::Update()
    {
        
    }
}
