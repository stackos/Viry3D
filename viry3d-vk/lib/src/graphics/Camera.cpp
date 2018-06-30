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
#include "Shader.h"

namespace Viry3D
{
    Camera::Camera():
        m_index(-1),
        m_render_pass_dirty(true),
        m_renderer_order_dirty(true),
        m_instance_cmds_dirty(true),
        m_projection_matrix_dirty(true),
        m_clear_flags(CameraClearFlags::ColorAndDepth),
        m_clear_color(0, 0, 0, 1),
        m_viewport_rect(0, 0, 1, 1),
        m_fov(45),
        m_aspect(1),
        m_near(0.3f),
        m_far(1000),
        m_orthographic(false),
        m_orthographic_size(1),
        m_depth(0),
        m_render_pass(nullptr),
        m_cmd_pool(nullptr)
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
        Display::Instance()->MarkPrimaryCmdDirty();
    }

    void Camera::SetViewportRect(const Rect& rect)
    {
        m_viewport_rect = rect;
        m_instance_cmds_dirty = true;
    }

    void Camera::SetFieldOfView(float fov)
    {
        m_fov = fov;
        m_projection_matrix_dirty = true;
    }

    void Camera::SetAspect(float aspect)
    {
        m_aspect = aspect;
        m_projection_matrix_dirty = true;
    }

    void Camera::SetNear(float near_clip)
    {
        m_near = near_clip;
        m_projection_matrix_dirty = true;
    }

    void Camera::SetFar(float far_clip)
    {
        m_far = far_clip;
        m_projection_matrix_dirty = true;
    }

    void Camera::SetOrthographic(bool ortho)
    {
        m_orthographic = ortho;
        m_projection_matrix_dirty = true;
    }

    void Camera::SetOrthographicSize(float size)
    {
        m_orthographic_size = size;
        m_projection_matrix_dirty = true;
    }

    void Camera::SetDepth(int depth)
    {
        m_depth = depth;
        Display::Instance()->MarkPrimaryCmdDirty();
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
            Display::Instance()->MarkPrimaryCmdDirty();
        }

        if (m_renderer_order_dirty)
        {
            m_renderer_order_dirty = false;
            this->SortRenderers();

            Display::Instance()->MarkPrimaryCmdDirty();
        }

        if (m_projection_matrix_dirty)
        {
            m_projection_matrix_dirty = false;

            this->UpdateMatrix();
        }
        
        this->UpdateRenderers();
        this->UpdateInstanceCmds();
    }

    void Camera::OnResize(int width, int height)
    {
        this->ClearRenderPass();
        this->ClearInstanceCmds();

        m_render_pass_dirty = true;
        m_instance_cmds_dirty = true;

        this->SetAspect(width / (float) height);
    }

    void Camera::UpdateRenderPass()
    {
        this->ClearRenderPass();

        Display::Instance()->CreateRenderPass(
            m_render_target_color,
            m_render_target_depth,
            m_clear_flags,
            &m_render_pass,
            m_framebuffers);
    }

    void Camera::ClearRenderPass()
    {
        VkDevice device = Display::Instance()->GetDevice();

        for (int i = 0; i < m_framebuffers.Size(); ++i)
        {
            vkDestroyFramebuffer(device, m_framebuffers[i], nullptr);
        }
        m_framebuffers.Clear();

        if (m_render_pass)
        {
            Shader::OnRenderPassDestroy(m_render_pass);
            vkDestroyRenderPass(device, m_render_pass, nullptr);
            m_render_pass = nullptr;
        }
    }

    void Camera::SortRenderers()
    {
        m_renderers.Sort([](const RendererInstance& a, const RendererInstance& b) {
            const Ref<Material>& ma = a.renderer->GetMaterial();
            const Ref<Material>& mb = b.renderer->GetMaterial();
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
        for (auto& i : m_renderers)
        {
            if (i.cmd == nullptr)
            {
                if (m_cmd_pool == nullptr)
                {
                    Display::Instance()->CreateCommandPool(&m_cmd_pool);
                }
                
                Display::Instance()->CreateCommandBuffer(m_cmd_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY, &i.cmd);
            }

            if (i.cmd_dirty || m_instance_cmds_dirty)
            {
                i.cmd_dirty = false;

                this->BuildInstanceCmd(i.cmd, i.renderer);
            }
        }

        m_instance_cmds_dirty = false;
    }

    void Camera::ClearInstanceCmds()
    {
        VkDevice device = Display::Instance()->GetDevice();

        for (auto& i : m_renderers)
        {
            if (i.cmd)
            {
                vkFreeCommandBuffers(device, m_cmd_pool, 1, &i.cmd);
                i.cmd = nullptr;
            }
        }

        if (m_cmd_pool)
        {
            vkDestroyCommandPool(device, m_cmd_pool, nullptr);
            m_cmd_pool = nullptr;
        }
    }

    Vector<VkCommandBuffer> Camera::GetInstanceCmds() const
    {
        Vector<VkCommandBuffer> cmds;

        for (const auto& i : m_renderers)
        {
            cmds.Add(i.cmd);
        }

        return cmds;
    }

    VkFramebuffer Camera::GetFramebuffer(int index) const
    {
        if (this->HasRenderTarget())
        {
            return m_framebuffers[0];
        }
        else
        {
            return m_framebuffers[index];
        }
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
            return Display::Instance()->GetWidth();
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
            return Display::Instance()->GetHeight();
        }
    }

    void Camera::AddRenderer(const Ref<Renderer>& renderer)
    {
        RendererInstance instance;
        instance.renderer = renderer;

        if (!m_renderers.Contains(instance))
        {
            m_renderers.AddLast(instance);
            this->MarkRendererOrderDirty();

            renderer->OnAddToCamera(this);
        }
    }

    void Camera::RemoveRenderer(const Ref<Renderer>& renderer)
    {
        VkDevice device = Display::Instance()->GetDevice();

        Display::Instance()->WaitDevice();

        for (auto i = m_renderers.begin(); i != m_renderers.end(); ++i)
        {
            if (i->renderer == renderer)
            {
                if (i->cmd)
                {
                    vkFreeCommandBuffers(device, m_cmd_pool, 1, &i->cmd);
                }
                m_renderers.Remove(i);
                break;
            }
        }

        Display::Instance()->MarkPrimaryCmdDirty();

        renderer->OnRemoveFromCamera(this);
    }

    void Camera::MarkRendererOrderDirty()
    {
        m_renderer_order_dirty = true;
    }

    void Camera::MarkInstanceCmdDirty(Renderer* renderer)
    {
        for (auto& i : m_renderers)
        {
            if (i.renderer.get() == renderer)
            {
                i.cmd_dirty = true;
                break;
            }
        }
    }

    void Camera::BuildInstanceCmd(VkCommandBuffer cmd, const Ref<Renderer>& renderer)
    {
        const Ref<Material>& material = renderer->GetMaterial();
        const Ref<Material>& instance_material = renderer->GetInstanceMaterial();
        const Ref<Shader>& shader = material->GetShader();
        const Vector<UniformSet>& sets_info = shader->GetUniformSets();

        int index_offset;
        int index_count;
        renderer->GetIndexRange(index_offset, index_count);

        Vector<VkDescriptorSet> descriptor_sets = material->GetDescriptorSets();
        
        if (instance_material)
        {
            const Vector<VkDescriptorSet>& instance_descriptor_sets = instance_material->GetDescriptorSets();
            const Map<String, Vector<MaterialProperty>>& instance_properties = instance_material->GetProperties();
            for (const auto& i : instance_properties)
            {
                int instance_set_index = instance_material->FindUniformSetIndex(i.second[0].name);
                if (instance_set_index >= 0)
                {
                    descriptor_sets[instance_set_index] = instance_descriptor_sets[instance_set_index];
                }
            }
        }

        bool color_attachment = true;
        bool depth_attachment = true;
        if (this->HasRenderTarget())
        {
            color_attachment = (bool) this->GetRenderTargetColor();
            depth_attachment = (bool) this->GetRenderTargetDepth();
        }

        Vector<Vector<int>> dynamic_offsets(descriptor_sets.Size());
        for (int i = 0; i < descriptor_sets.Size(); ++i)
        {
            int buffer_count = sets_info[i].buffers.Size();
            for (int j = 0; j < buffer_count; ++j)
            {
                dynamic_offsets[i].Add(sets_info[i].buffers[j].offset_alignment * m_index);
            }
        }

        Display::Instance()->BuildInstanceCmd(
            cmd,
            m_render_pass,
            shader->GetPipelineLayout(),
            shader->GetPipeline(m_render_pass, color_attachment, depth_attachment),
            descriptor_sets,
            dynamic_offsets,
            this->GetTargetWidth(),
            this->GetTargetHeight(),
            m_viewport_rect,
            renderer->GetVertexBuffer(),
            renderer->GetIndexBuffer(),
            index_offset,
            index_count);
    }

    void Camera::UpdateRenderers()
    {
        for (auto& i : m_renderers)
        {
            i.renderer->Update();
        }
    }

    void Camera::UpdateMatrix()
    {
        if (m_projection_matrix_dirty)
        {
            m_projection_matrix_dirty = false;

            if (m_orthographic)
            {

            }
            else
            {

            }
        }
    }
}
