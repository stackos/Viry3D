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
		m_camera(nullptr),
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

    void Renderer::SetMaterial(const Ref<Material>& material)
    {
        if (m_material)
        {
            m_material->OnUnSetRenderer(this);
        }

        m_material = material;
        this->MarkRendererOrderDirty();

        if (m_material)
        {
            m_material->OnSetRenderer(this);
        }

#if VR_VULKAN
        this->MarkInstanceCmdDirty();
#endif

        if (m_instance_material)
        {
            if (m_material)
            {
                Map<String, MaterialProperty> properties = m_instance_material->GetProperties();

                m_instance_material = RefMake<Material>(m_material->GetShader());
                for (const auto& i : properties)
                {
                    switch (i.second.type)
                    {
                        case MaterialProperty::Type::Color:
                            m_instance_material->SetColor(i.second.name, *(Color*) &i.second.data);
                            break;
                        case MaterialProperty::Type::Vector:
                            m_instance_material->SetVector(i.second.name, *(Vector4*) &i.second.data);
                            break;
                        case MaterialProperty::Type::Float:
                            m_instance_material->SetFloat(i.second.name, *(float*) &i.second.data);
                            break;
                        case MaterialProperty::Type::Texture:
                            m_instance_material->SetTexture(i.second.name, i.second.texture);
                            break;
                        case MaterialProperty::Type::Matrix:
                            m_instance_material->SetMatrix(i.second.name, *(Matrix4x4*) &i.second.data);
                            break;
                        case MaterialProperty::Type::VectorArray:
                            m_instance_material->SetVectorArray(i.second.name, i.second.vector_array);
                            break;
                        case MaterialProperty::Type::Int:
                            m_instance_material->SetInt(i.second.name, *(int*) &i.second.data);
                            break;
                    }
                }
            }
            else
            {
                m_instance_material.reset();
            }
        }

        if (m_camera)
        {
            m_camera->SetViewUniforms(m_material);
            m_camera->SetProjectionUniform(m_material);
        }

        m_model_matrix_dirty = true;
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

    void Renderer::OnAddToCamera(Camera* camera)
    {
		assert(m_camera == nullptr);
		m_camera = camera;
    }

    void Renderer::OnRemoveFromCamera(Camera* camera)
    {
		assert(m_camera == camera);
		m_camera = nullptr;
    }

    void Renderer::MarkRendererOrderDirty()
    {
		if (m_camera)
		{
			m_camera->MarkRendererOrderDirty();
		}
    }

#if VR_VULKAN
    void Renderer::MarkInstanceCmdDirty()
    {
		if (m_camera)
		{
			m_camera->MarkInstanceCmdDirty(this);
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
        if (m_material)
        {
            m_material->UpdateUniformSets();
        }

        if (m_instance_material)
        {
            m_instance_material->UpdateUniformSets();
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
        if (m_material)
        {
            if (!m_instance_material)
            {
                m_instance_material = RefMake<Material>(m_material->GetShader());
            }
            
            m_instance_material->SetMatrix(name, value);
        }
    }

    void Renderer::SetInstanceVectorArray(const String& name, const Vector<Vector4>& value)
    {
        if (m_material)
        {
            if (!m_instance_material)
            {
                m_instance_material = RefMake<Material>(m_material->GetShader());
            }

            m_instance_material->SetVectorArray(name, value);
        }
    }

    void Renderer::SetInstanceVector(const String& name, const Vector4& value)
    {
        if (m_material)
        {
            if (!m_instance_material)
            {
                m_instance_material = RefMake<Material>(m_material->GetShader());
            }

            m_instance_material->SetVector(name, value);
        }
    }

    void Renderer::SetInstanceInt(const String& name, int value)
    {
        if (m_material)
        {
            if (!m_instance_material)
            {
                m_instance_material = RefMake<Material>(m_material->GetShader());
            }

            m_instance_material->SetInt(name, value);
        }
    }

#if VR_GLES
    void Renderer::OnDraw()
    {
        const Ref<Material>& material = this->GetMaterial();
        Ref<BufferObject> vertex_buffer = this->GetVertexBuffer();
        Ref<BufferObject> index_buffer = this->GetIndexBuffer();
        const DrawBuffer& draw_buffer = this->GetDrawBuffer();

        if (!material || !vertex_buffer || !index_buffer || draw_buffer.index_count == 0)
        {
            return;
        }

        const Ref<Shader>& shader = material->GetShader();
        if (!shader->Use())
        {
            return;
        }

        vertex_buffer->Bind();
        index_buffer->Bind();
        shader->EnableVertexAttribs();
        shader->ApplyRenderState();
        material->ApplyUniforms();

        const Ref<Material>& instance_material = this->GetInstanceMaterial();
        if (instance_material)
        {
            instance_material->ApplyUniforms();
        }

        glDrawElements(GL_TRIANGLES, draw_buffer.index_count, GL_UNSIGNED_SHORT, (const void*) (draw_buffer.first_index * sizeof(unsigned short)));

        shader->DisableVertexAttribs();
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
