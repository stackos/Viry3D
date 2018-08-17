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

#include "Node.h"
#include "Display.h"
#include "CameraClearFlags.h"
#include "Color.h"
#include "math/Rect.h"
#include "math/Matrix4x4.h"
#include "container/Vector.h"
#include "container/List.h"

namespace Viry3D
{
    class Texture;
    class Renderer;

    struct RendererInstance
    {
        Ref<Renderer> renderer;
        bool cmd_dirty = true;
        VkCommandBuffer cmd = VK_NULL_HANDLE;

        bool operator ==(const RendererInstance& a) const
        {
            return this->renderer == a.renderer;
        }
    };

    class Camera : public Node
    {
    public:
        Camera();
        virtual ~Camera();

        CameraClearFlags GetClearFlags() const { return m_clear_flags; }
        void SetClearFlags(CameraClearFlags flags);
        const Color& GetClearColor() const { return m_clear_color; }
        void SetClearColor(const Color& color);
        const Rect& GetViewportRect() const { return m_viewport_rect; }
        void SetViewportRect(const Rect& rect);
        int GetDepth() const { return m_depth; }
        void SetDepth(int depth);
        bool HasRenderTarget() const { return m_render_target_color || m_render_target_depth; }
        const Ref<Texture>& GetRenderTargetColor() const { return m_render_target_color; }
        const Ref<Texture>& GetRenderTargetDepth() const { return m_render_target_depth; }
        void SetRenderTarget(const Ref<Texture>& color_texture, const Ref<Texture>& depth_texture);
        void Update();
        void OnFrameEnd();
        void OnResize(int width, int height);
        void OnPause();
        VkRenderPass GetRenderPass() const { return m_render_pass; }
        VkFramebuffer GetFramebuffer(int index) const;
        int GetTargetWidth() const;
        int GetTargetHeight() const;
        void AddRenderer(const Ref<Renderer>& renderer);
        void RemoveRenderer(const Ref<Renderer>& renderer);
        void MarkRendererOrderDirty();
        void MarkInstanceCmdDirty(Renderer* renderer);
        Vector<VkCommandBuffer> GetInstanceCmds() const;
        float GetFieldOfView() const { return m_field_of_view; }
        void SetFieldOfView(float fov);
        float GetNearClip() const { return m_near_clip; }
        void SetNearClip(float clip);
        float GetFarClip() const { return m_far_clip; }
        void SetFarClip(float clip);
        bool IsOthographic() const { return m_othographic; }
        void SetOthographic(bool enable);
        float GetOthographicSize() const { return m_othographic_size; }
        void SetOthographicSize(float size);
        const Matrix4x4& GetViewMatrix();
        const Matrix4x4& GetProjectionMatrix();

    protected:
        virtual void OnMatrixDirty();

    private:
        void UpdateRenderPass();
        void ClearRenderPass();
        void SortRenderers();
        void UpdateInstanceCmds();
        void ClearInstanceCmds();
        void BuildInstanceCmd(VkCommandBuffer cmd, const Ref<Renderer>& renderer);
        void UpdateRenderers();

    private:
        bool m_render_pass_dirty;
        bool m_renderer_order_dirty;
        bool m_instance_cmds_dirty;
        CameraClearFlags m_clear_flags;
        Color m_clear_color;
        Rect m_viewport_rect;
        int m_depth;
        Ref<Texture> m_render_target_color;
        Ref<Texture> m_render_target_depth;
        VkRenderPass m_render_pass;
        Vector<VkFramebuffer> m_framebuffers;
        List<RendererInstance> m_renderers;
        VkCommandPool m_cmd_pool;
        Matrix4x4 m_view_matrix;
        bool m_view_matrix_dirty;
        Matrix4x4 m_projection_matrix;
        bool m_projection_matrix_dirty;
        float m_field_of_view;
        float m_near_clip;
        float m_far_clip;
        bool m_othographic;
        float m_othographic_size;
    };
}
