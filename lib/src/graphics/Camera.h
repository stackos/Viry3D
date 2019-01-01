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
    class Computer;

    struct RendererInstance
    {
        Ref<Renderer> renderer;
#if VR_VULKAN
        bool cmd_dirty = true;
        VkCommandBuffer cmd = VK_NULL_HANDLE;
        VkCommandBuffer compute_cmd = VK_NULL_HANDLE;
#endif

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
        const Vector<Ref<Texture>>& GetExtraRenderTargets() const { return m_extra_render_targets; }
        void SetExtraRenderTargets(const Vector<Ref<Texture>>& color_textures);
        void Update();
        void OnFrameEnd();
        void OnResize(int width, int height);
        void OnPause();
        int GetTargetWidth() const;
        int GetTargetHeight() const;
        void AddRenderer(const Ref<Renderer>& renderer);
        void RemoveRenderer(const Ref<Renderer>& renderer);
        float GetFieldOfView() const { return m_field_of_view; }
        void SetFieldOfView(float fov);
        float GetNearClip() const { return m_near_clip; }
        void SetNearClip(float clip);
        float GetFarClip() const { return m_far_clip; }
        void SetFarClip(float clip);
        bool IsOrthographic() const { return m_orthographic; }
        void SetOrthographic(bool enable);
        float GetOrthographicSize() const { return m_orthographic_size; }
        void SetOrthographicSize(float size);
        bool IsStereoRendering() const { return m_stereo_rendering; }
        void SetStereoRendering(bool enable);
        void SetStereoOffset(float offset);
        void SetViewMatrixExternal(const Matrix4x4& mat);
        void SetProjectionMatrixExternal(const Matrix4x4& mat);
        const Matrix4x4& GetViewMatrix();
        const Matrix4x4& GetProjectionMatrix();
        void MarkRendererOrderDirty();
        void SetViewUniforms(const Ref<Material>& material);
        void SetProjectionUniform(const Ref<Material>& material);
#if VR_VULKAN
        void MarkInstanceCmdDirty(Renderer* renderer);
        VkRenderPass GetRenderPass() const { return m_render_pass; }
        VkFramebuffer GetFramebuffer(int index) const;
        Vector<VkCommandBuffer> GetInstanceCmds() const;
        Vector<VkCommandBuffer> GetComputeInstanceCmds() const;
#elif VR_GLES
        void OnDraw();
#endif

    protected:
        virtual void OnMatrixDirty();

    private:
        void SortRenderers();
        void UpdateRenderers();
#if VR_VULKAN
        void UpdateRenderPass();
        void ClearRenderPass();
        void UpdateInstanceCmds();
        void ClearInstanceCmds();
        void BuildInstanceCmd(VkCommandBuffer cmd, const Ref<Renderer>& renderer);
        void BuildComputeInstanceCmd(VkCommandBuffer cmd, const Ref<Computer>& computer);
#elif VR_GLES
        void BindTarget();
        void ClearTarget();
        void ResolveMultiSample();
#endif

    private:
#if VR_VULKAN
        VkRenderPass m_render_pass;
        Vector<VkFramebuffer> m_framebuffers;
        VkCommandPool m_cmd_pool;
        VkCommandPool m_compute_cmd_pool;
        bool m_render_pass_dirty;
        bool m_instance_cmds_dirty;
#elif VR_GLES
        GLuint m_framebuffer;
        GLuint m_framebuffer_resolve;
#endif
        bool m_renderer_order_dirty;
        CameraClearFlags m_clear_flags;
        Color m_clear_color;
        Rect m_viewport_rect;
        int m_depth;
        Ref<Texture> m_render_target_color;
        Ref<Texture> m_render_target_depth;
        Vector<Ref<Texture>> m_extra_render_targets;
        List<RendererInstance> m_renderers;
        Matrix4x4 m_view_matrix;
        bool m_view_matrix_dirty;
        Matrix4x4 m_projection_matrix;
        bool m_projection_matrix_dirty;
        float m_field_of_view;
        float m_near_clip;
        float m_far_clip;
        bool m_orthographic;
        float m_orthographic_size;
        bool m_stereo_rendering;
        float m_stereo_offset;
        bool m_view_matrix_external;
        bool m_projection_matrix_external;
    };
}
