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

#include "Renderer.h"
#include "Camera.h"
#include "Material.h"
#include "Shader.h"
#include "BufferObject.h"
#include "Debug.h"

namespace Viry3D
{
    Renderer::Renderer():
        m_draw_buffer_dirty(true),
        m_model_matrix_dirty(true),
        m_instance_buffer_dirty(false),
        m_instance_extra_vector_count(0),
        m_lightmap_scale_offset(1, 1, 0, 0),
        m_lightmap_index(-1),
        m_lightmap_uv_dirty(false)
    {
    
    }

    Renderer::~Renderer()
    {
#if VR_VULKAN
        if (m_instance_buffer)
        {
            m_instance_buffer->Destroy(Display::Instance()->GetDevice());
        }
#endif

        m_instance_buffer.reset();
    }

    Ref<Material> Renderer::GetMaterial() const
    {
        Ref<Material> material;

        if (m_materials.Size() > 0)
        {
            material = m_materials[0];
        }

        return material;
    }

    void Renderer::SetMaterial(const Ref<Material>& material)
    {
        this->SetMaterials({ material });
    }

    void Renderer::SetMaterials(const Vector<Ref<Material>>& materials)
    {
        for (auto& i : m_materials)
        {
            if (i)
            {
                i->OnUnSetRenderer(this);
            }
        }

        m_materials = materials;
        this->MarkRendererOrderDirty();

        for (auto& i : m_materials)
        {
            if (i)
            {
                i->OnSetRenderer(this);
            }
        }

#if VR_VULKAN
        this->MarkInstanceCmdDirty();
#endif

        if (!m_camera.expired())
        {
            for (auto& i : m_materials)
            {
                if (i)
                {
                    m_camera.lock()->SetViewUniforms(i);
                    m_camera.lock()->SetProjectionUniform(i);
                }
            }
        }

        m_instance_materials.Clear();

        m_model_matrix_dirty = true;
        if (m_lightmap_index >= 0)
        {
            m_lightmap_uv_dirty = true;
        }
        m_draw_buffer_dirty = true;
    }

    void Renderer::SetLightmapIndex(int index)
    {
        m_lightmap_index = index;
        m_lightmap_uv_dirty = true;
    }

    void Renderer::SetLightmapScaleOffset(const Vector4& vec)
    {
        m_lightmap_scale_offset = vec;
        m_lightmap_uv_dirty = true;
    }

    void Renderer::OnAddToCamera(const Ref<Camera>& camera)
    {
		assert(m_camera.expired());
		m_camera = camera;
    }

    void Renderer::OnRemoveFromCamera(const Ref<Camera>& camera)
    {
		assert(m_camera.lock() == camera);
		m_camera.reset();
    }

    void Renderer::MarkRendererOrderDirty()
    {
		if (!m_camera.expired())
		{
			m_camera.lock()->MarkRendererOrderDirty();
		}
    }

#if VR_VULKAN
    void Renderer::MarkInstanceCmdDirty()
    {
		if (!m_camera.expired())
		{
			m_camera.lock()->MarkInstanceCmdDirty(this);
		}
    }
#endif

    void Renderer::OnMatrixDirty()
    {
        m_model_matrix_dirty = true;
    }

    void Renderer::Update()
    {
        if (m_model_matrix_dirty)
        {
            m_model_matrix_dirty = false;
            this->SetInstanceMatrix(MODEL_MATRIX, this->GetLocalToWorldMatrix());
        }

        if (m_lightmap_uv_dirty && m_lightmap_index >= 0)
        {
            m_lightmap_uv_dirty = false;
            this->SetInstanceVector(LIGHTMAP_SCALE_OFFSET, m_lightmap_scale_offset);
            this->SetInstanceInt(LIGHTMAP_INDEX, m_lightmap_index);
        }

#if VR_VULKAN
        for (auto& i : m_materials)
        {
            if (i)
            {
                i->UpdateUniformSets();
            }
        }

        for (auto& i : m_instance_materials)
        {
            if (i)
            {
                i->UpdateUniformSets();
            }
        }
#endif

        if (m_instance_buffer_dirty)
        {
            m_instance_buffer_dirty = false;
            this->UpdateInstanceBuffer();
        }

        if (m_draw_buffer_dirty)
        {
            m_draw_buffer_dirty = false;
            this->UpdateDrawBuffer();
        }
    }

    void Renderer::SetInstanceMatrix(const String& name, const Matrix4x4& value)
    {
        if (m_instance_materials.Size() != m_materials.Size())
        {
            m_instance_materials.Resize(m_materials.Size());
        }

        for (int i = 0; i < m_materials.Size(); ++i)
        {
            if (m_materials[i])
            {
                if (!m_instance_materials[i])
                {
                    m_instance_materials[i] = RefMake<Material>(m_materials[i]->GetShader());
                }

                m_instance_materials[i]->SetMatrix(name, value);
            }
        }
    }

    void Renderer::SetInstanceVectorArray(const String& name, const Vector<Vector4>& value)
    {
        if (m_instance_materials.Size() != m_materials.Size())
        {
            m_instance_materials.Resize(m_materials.Size());
        }

        for (int i = 0; i < m_materials.Size(); ++i)
        {
            if (m_materials[i])
            {
                if (!m_instance_materials[i])
                {
                    m_instance_materials[i] = RefMake<Material>(m_materials[i]->GetShader());
                }

                m_instance_materials[i]->SetVectorArray(name, value);
            }
        }
    }

    void Renderer::SetInstanceVector(const String& name, const Vector4& value)
    {
        if (m_instance_materials.Size() != m_materials.Size())
        {
            m_instance_materials.Resize(m_materials.Size());
        }

        for (int i = 0; i < m_materials.Size(); ++i)
        {
            if (m_materials[i])
            {
                if (!m_instance_materials[i])
                {
                    m_instance_materials[i] = RefMake<Material>(m_materials[i]->GetShader());
                }

                m_instance_materials[i]->SetVector(name, value);
            }
        }
    }

    void Renderer::SetInstanceInt(const String& name, int value)
    {
        if (m_instance_materials.Size() != m_materials.Size())
        {
            m_instance_materials.Resize(m_materials.Size());
        }

        for (int i = 0; i < m_materials.Size(); ++i)
        {
            if (m_materials[i])
            {
                if (!m_instance_materials[i])
                {
                    m_instance_materials[i] = RefMake<Material>(m_materials[i]->GetShader());
                }

                m_instance_materials[i]->SetInt(name, value);
            }
        }
    }

#if VR_GLES
    void Renderer::OnDraw()
    {
        const auto& materials = this->GetMaterials();
        const auto& instance_materials = this->GetInstanceMaterials();
        Ref<BufferObject> vertex_buffer = this->GetVertexBuffer();
        Ref<BufferObject> index_buffer = this->GetIndexBuffer();
        const auto& draw_buffers = this->GetDrawBuffers();

        if (materials.Size() == 0 || !vertex_buffer || !index_buffer || draw_buffers.Size() == 0)
        {
            return;
        }

        vertex_buffer->Bind();
        index_buffer->Bind();

        for (int i = 0; i < materials.Size(); ++i)
        {
            const auto& material = materials[i];
            const Ref<Shader>& shader = material->GetShader();
            if (!shader->Use())
            {
                continue;
            }

            if (draw_buffers[i].index_count == 0)
            {
                continue;
            }

            shader->EnableVertexAttribs();
            shader->ApplyRenderState();
            material->ApplyUniforms();

            if (i < instance_materials.Size() && instance_materials[i])
            {
                instance_materials[i]->ApplyUniforms();
            }

            const Vector4* clip_rect = material->GetVector(CLIP_RECT);
            if (clip_rect)
            {
                glEnable(GL_SCISSOR_TEST);
                int target_width = m_camera.lock()->GetTargetWidth();
                int target_height = m_camera.lock()->GetTargetHeight();
                glScissor((int) (clip_rect->x * target_width),
                    (int) ((1.0f - clip_rect->y - clip_rect->w) * target_height),
                    (int) (clip_rect->z * target_width),
                    (int) (clip_rect->w * target_height));
            }
            else
            {
                glDisable(GL_SCISSOR_TEST);
            }

            glDrawElements(GL_TRIANGLES, draw_buffers[i].index_count, GL_UNSIGNED_SHORT, (const void*) (draw_buffers[i].first_index * sizeof(unsigned short)));

            if (clip_rect)
            {
                glDisable(GL_SCISSOR_TEST);
            }

            shader->DisableVertexAttribs();
        }
        
        vertex_buffer->Unind();
        index_buffer->Unind();

        LogGLError();
    }
#endif

    void Renderer::AddInstance(const Vector3& pos, const Quaternion& rot, const Vector3& scale)
    {
        if (m_instances.Empty())
        {
            m_instances.Resize(1);

            m_instances[0].position = Vector3(0, 0, 0);
            m_instances[0].rotation = Quaternion::Identity();
            m_instances[0].scale = Vector3(1, 1, 1);
        }

        RendererInstanceTransform instacne;
        instacne.position = pos;
        instacne.rotation = rot;
        instacne.scale = scale;

        m_instances.Add(instacne);

        m_instance_buffer_dirty = true;
        m_draw_buffer_dirty = true;

#if VR_VULKAN
        this->MarkInstanceCmdDirty();
#endif
    }

    void Renderer::SetInstanceTransform(int instance_index, const Vector3& pos, const Quaternion& rot, const Vector3& scale)
    {
        RendererInstanceTransform &instacne = m_instances[instance_index];
        instacne.position = pos;
        instacne.rotation = rot;
        instacne.scale = scale;

        m_instance_buffer_dirty = true;
    }

    void Renderer::SetInstanceExtraVector(int instance_index, int vector_index, const Vector4& v)
    {
        RendererInstanceTransform &instacne = m_instances[instance_index];

        if (instacne.verctors.Size() < vector_index + 1)
        {
            instacne.verctors.Resize(vector_index + 1);
            if (m_instance_extra_vector_count < instacne.verctors.Size())
            {
                m_instance_extra_vector_count = instacne.verctors.Size();
            }
        }
        instacne.verctors[vector_index] = v;

        m_instance_buffer_dirty = true;
    }

    int Renderer::GetInstanceCount() const
    {
        int count = m_instances.Size();

        if (count == 0)
        {
            count = 1;
        }

        return count;
    }

    int Renderer::GetInstanceStride() const
    {
        return sizeof(Vector4) * (4 + m_instance_extra_vector_count);
    }

    void Renderer::UpdateInstanceBuffer()
    {
        int instance_count = this->GetInstanceCount();
        if (instance_count > 1)
        {
            Vector<Vector4> vectors(instance_count * (4 + m_instance_extra_vector_count));
            for (int i = 0; i < instance_count; ++i)
            {
                Matrix4x4* mat = (Matrix4x4*) &vectors[i * (4 + m_instance_extra_vector_count) + 0];
                *mat = Matrix4x4::TRS(m_instances[i].position, m_instances[i].rotation, m_instances[i].scale);

                for (int j = 0; j < m_instances[i].verctors.Size(); ++j)
                {
                    vectors[i * (4 + m_instance_extra_vector_count) + 4 + j] = m_instances[i].verctors[j];
                }
            }

#if VR_VULKAN
            int buffer_size = vectors.SizeInBytes();
            
            if (!m_instance_buffer || m_instance_buffer->GetSize() < buffer_size)
            {
                if (m_instance_buffer)
                {
                    m_instance_buffer->Destroy(Display::Instance()->GetDevice());
                    m_instance_buffer.reset();
                }
                m_instance_buffer = Display::Instance()->CreateBuffer(vectors.Bytes(), buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, true, VK_FORMAT_UNDEFINED);

                this->MarkInstanceCmdDirty();
            }
            else
            {
                Display::Instance()->UpdateBuffer(m_instance_buffer, 0, vectors.Bytes(), buffer_size);
            }
#endif
        }
    }
}
