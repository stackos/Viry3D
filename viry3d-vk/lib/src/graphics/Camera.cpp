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
#include "Texture.h"
#include "Renderer.h"
#include "Material.h"

namespace Viry3D
{
    Camera::Camera():
        m_render_pass_dirty(true),
        m_renderer_order_dirty(true),
        m_instance_cmds_dirty(true),
        m_clear_flags(CameraClearFlags::ColorAndDepth),
        m_clear_color(0, 0, 0, 1),
        m_viewport_rect(0, 0, 1, 1)
    {
    
    }

    Camera::~Camera()
    {
        this->ClearRenderPass();
        this->ClearInstanceCmds();
    }

    void Camera::SetClearFlags(CameraClearFlags flags)
    {
        m_clear_flags = flags;
        m_render_pass_dirty = true;
    }

    void Camera::SetClearColor(const Color& color)
    {
        m_clear_color = color;
        Display::GetDisplay()->MarkPrimaryCmdDirty();
    }

    void Camera::SetViewportRect(const Rect& rect)
    {
        m_viewport_rect = rect;
        m_instance_cmds_dirty = true;
    }

    void Camera::SetRenderTarget(const Ref<Texture>& color_texture, const Ref<Texture>& depth_texture)
    {
        m_render_target_color = color_texture;
        m_render_target_depth = depth_texture;
        m_render_pass_dirty = true;
    }

    void Camera::Update()
    {
        if (m_render_pass_dirty)
        {
            m_render_pass_dirty = false;
            this->UpdateRenderPass();

            m_instance_cmds_dirty = true;
            Display::GetDisplay()->MarkPrimaryCmdDirty();
        }

        if (m_renderer_order_dirty)
        {
            m_renderer_order_dirty = false;
            this->SortRenderers();

            Display::GetDisplay()->MarkPrimaryCmdDirty();
        }

        if (m_instance_cmds_dirty)
        {
            m_instance_cmds_dirty = false;
            this->UpdateInstanceCmds();

            Display::GetDisplay()->MarkPrimaryCmdDirty();
        }
    }

    void Camera::OnResize(int width, int height)
    {
        this->ClearRenderPass();
        this->ClearInstanceCmds();

        m_render_pass_dirty = true;
        m_instance_cmds_dirty = true;
    }

    void Camera::UpdateRenderPass()
    {
        this->ClearRenderPass();

        Display::GetDisplay()->CreateRenderPass(
            m_render_target_color,
            m_render_target_depth,
            m_clear_flags,
            &m_render_pass,
            m_framebuffers);
    }

    void Camera::ClearRenderPass()
    {
        VkDevice device = Display::GetDisplay()->GetDevice();

        for (int i = 0; i < m_framebuffers.Size(); ++i)
        {
            vkDestroyFramebuffer(device, m_framebuffers[i], nullptr);
        }
        m_framebuffers.Clear();

        if (m_render_pass)
        {
            vkDestroyRenderPass(device, m_render_pass, nullptr);
            m_render_pass = nullptr;
        }
    }

    void Camera::SortRenderers()
    {
        m_renderers.Sort([](const Ref<Renderer>& a, const Ref<Renderer>& b) {
            const Ref<Material>& ma = a->GetMaterial();
            const Ref<Material>& mb = b->GetMaterial();
            if (ma && mb)
            {
                return ma->GetQueue() < mb->GetQueue();
            }
            else if (!ma && mb)
            {
                return true;
            }
            else
            {
                return false;
            }
        });
    }

    void Camera::UpdateInstanceCmds()
    {
        this->ClearInstanceCmds();

        // rebuild all renderer instance cmds
    }

    void Camera::ClearInstanceCmds()
    {
        
    }

    int Camera::GetTargetWidth() const
    {
        if (m_render_target_color)
        {
            return m_render_target_color->GetWidth();
        }
        else if (m_render_target_depth)
        {
            return m_render_target_depth->GetWidth();
        }
        else
        {
            return Display::GetDisplay()->GetWidth();
        }
    }
    
    int Camera::GetTargetHeight() const
    {
        if (m_render_target_color)
        {
            return m_render_target_color->GetHeight();
        }
        else if (m_render_target_depth)
        {
            return m_render_target_depth->GetHeight();
        }
        else
        {
            return Display::GetDisplay()->GetHeight();
        }
    }

    void Camera::AddRenderer(const Ref<Renderer>& renderer)
    {
        if (!m_renderers.Contains(renderer))
        {
            m_renderers.AddLast(renderer);
            this->MarkRendererOrderDirty();
        }
    }

    void Camera::RemoveRenderer(const Ref<Renderer>& renderer)
    {
        m_renderers.Remove(renderer);
        Display::GetDisplay()->MarkPrimaryCmdDirty();
    }

    void Camera::MarkRendererOrderDirty()
    {
        m_renderer_order_dirty = true;
    }
}
