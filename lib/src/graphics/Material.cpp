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

#include "Material.h"
#include "Shader.h"
#include "Renderer.h"
#include "Light.h"
#include "BufferObject.h"
#include "Texture.h"

namespace Viry3D
{
    Material::Material(const Ref<Shader>& shader):
        m_shader(shader)
    {
#if VR_VULKAN
        m_shader->CreateDescriptorSets(m_descriptor_sets, m_uniform_sets);
#endif
    }

    Material::~Material()
    {
        this->Release();
    }

    void Material::Release()
    {
#if VR_VULKAN
        VkDevice device = Display::Instance()->GetDevice();

        m_descriptor_sets.Clear();

        for (int i = 0; i < m_uniform_sets.Size(); ++i)
        {
            for (int j = 0; j < m_uniform_sets[i].buffers.Size(); ++j)
            {
                if (m_uniform_sets[i].buffers[j].buffer)
                {
                    m_uniform_sets[i].buffers[j].buffer->Destroy(device);
                    m_uniform_sets[i].buffers[j].buffer.reset();
                }
            }
        }
        m_uniform_sets.Clear();
#endif
    }
    
    void Material::SetShader(const Ref<Shader>& shader)
    {
        this->Release();

        m_shader = shader;

#if VR_VULKAN
        m_shader->CreateDescriptorSets(m_descriptor_sets, m_uniform_sets);

        this->MarkInstanceCmdDirty();
#endif
    }

    int Material::GetQueue() const
    {
        if (m_queue)
        {
            return *m_queue;
        }

        return m_shader->GetRenderState().queue;
    }
    
    void Material::SetQueue(int queue)
    {
        m_queue = RefMake<int>(queue);

        this->MarkRendererOrderDirty();
    }

    void Material::OnSetRenderer(Renderer* renderer)
    {
        m_renderers.AddLast(renderer);
    }

    void Material::OnUnSetRenderer(Renderer* renderer)
    {
        m_renderers.Remove(renderer);
    }

    const Matrix4x4* Material::GetMatrix(const String& name) const
    {
        return this->GetProperty<Matrix4x4>(name, MaterialProperty::Type::Matrix);
    }

    void Material::SetMatrix(const String& name, const Matrix4x4& value)
    {
        this->SetProperty(name, value, MaterialProperty::Type::Matrix);
    }

    void Material::SetVector(const String& name, const Vector4& value)
    {
        this->SetProperty(name, value, MaterialProperty::Type::Vector);
    }

    void Material::SetColor(const String& name, const Color& value)
    {
        this->SetProperty(name, value, MaterialProperty::Type::Color);
    }

    void Material::SetFloat(const String& name, float value)
    {
        this->SetProperty(name, value, MaterialProperty::Type::Float);
    }

    void Material::SetInt(const String& name, int value)
    {
        this->SetProperty(name, value, MaterialProperty::Type::Int);
    }

    Ref<Texture> Material::GetTexture(const String& name) const
    {
        Ref<Texture> texture;
        const MaterialProperty* property_ptr;
        if (m_properties.TryGet(name, &property_ptr))
        {
            if (property_ptr->type == MaterialProperty::Type::Texture)
            {
                texture = property_ptr->texture;
            }
        }
        return texture;
    }

    void Material::SetTexture(const String& name, const Ref<Texture>& texture)
    {
        MaterialProperty* property_ptr;
        if (m_properties.TryGet(name, &property_ptr))
        {
            property_ptr->type = MaterialProperty::Type::Texture;
            property_ptr->texture = texture;
            property_ptr->dirty = true;
        }
        else
        {
            MaterialProperty property;
            property.name = name;
            property.type = MaterialProperty::Type::Texture;
            property.texture = texture;
            property.dirty = true;
            m_properties.Add(name, property);
        }
    }

    void Material::SetStorageBuffer(const String& name, const Ref<BufferObject>& buffer)
    {
        MaterialProperty* property_ptr;
        if (m_properties.TryGet(name, &property_ptr))
        {
            property_ptr->type = MaterialProperty::Type::StorageBuffer;
            property_ptr->buffer = buffer;
            property_ptr->dirty = true;
        }
        else
        {
            MaterialProperty property;
            property.name = name;
            property.type = MaterialProperty::Type::StorageBuffer;
            property.buffer = buffer;
            property.dirty = true;
            m_properties.Add(name, property);
        }
    }

    void Material::SetUniformTexelBuffer(const String& name, const Ref<BufferObject>& buffer)
    {
        MaterialProperty* property_ptr;
        if (m_properties.TryGet(name, &property_ptr))
        {
            property_ptr->type = MaterialProperty::Type::UniformTexelBuffer;
            property_ptr->buffer = buffer;
            property_ptr->dirty = true;
        }
        else
        {
            MaterialProperty property;
            property.name = name;
            property.type = MaterialProperty::Type::UniformTexelBuffer;
            property.buffer = buffer;
            property.dirty = true;
            m_properties.Add(name, property);
        }
    }

    void Material::SetStorageTexelBuffer(const String& name, const Ref<BufferObject>& buffer)
    {
        MaterialProperty* property_ptr;
        if (m_properties.TryGet(name, &property_ptr))
        {
            property_ptr->type = MaterialProperty::Type::StorageTexelBuffer;
            property_ptr->buffer = buffer;
            property_ptr->dirty = true;
        }
        else
        {
            MaterialProperty property;
            property.name = name;
            property.type = MaterialProperty::Type::StorageTexelBuffer;
            property.buffer = buffer;
            property.dirty = true;
            m_properties.Add(name, property);
        }
    }

    void Material::SetVectorArray(const String& name, const Vector<Vector4>& array)
    {
        MaterialProperty* property_ptr;
        if (m_properties.TryGet(name, &property_ptr))
        {
            property_ptr->type = MaterialProperty::Type::VectorArray;
            property_ptr->vector_array = array;
            property_ptr->dirty = true;
        }
        else
        {
            MaterialProperty property;
            property.name = name;
            property.type = MaterialProperty::Type::VectorArray;
            property.vector_array = array;
            property.dirty = true;
            m_properties.Add(name, property);
        }
    }

    void Material::SetMatrixArray(const String& name, const Vector<Matrix4x4>& array)
    {
        MaterialProperty* property_ptr;
        if (m_properties.TryGet(name, &property_ptr))
        {
            property_ptr->type = MaterialProperty::Type::MatrixArray;
            property_ptr->matrix_array = array;
            property_ptr->dirty = true;
        }
        else
        {
            MaterialProperty property;
            property.name = name;
            property.type = MaterialProperty::Type::MatrixArray;
            property.matrix_array = array;
            property.dirty = true;
            m_properties.Add(name, property);
        }
    }

    void Material::SetLightProperties(const Ref<Light>& light)
    {
        this->SetColor(AMBIENT_COLOR, Light::GetAmbientColor());
        if (light->GetType() == LightType::Directional)
        {
            this->SetVector(LIGHT_POSITION, light->GetForward());
        }
        else
        {
            this->SetVector(LIGHT_POSITION, light->GetPosition());
        }
        this->SetColor(LIGHT_COLOR, light->GetColor());
        this->SetFloat(LIGHT_ITENSITY, light->GetIntensity());
    }

    void Material::MarkRendererOrderDirty()
    {
        for (auto i : m_renderers)
        {
            i->MarkRendererOrderDirty();
        }
    }

#if VR_VULKAN
    void Material::MarkInstanceCmdDirty()
    {
        for (auto i : m_renderers)
        {
            i->MarkInstanceCmdDirty();
        }
    }

    void Material::UpdateUniformSets()
    {
        bool instance_cmd_dirty = false;

        for (auto& i : m_properties)
        {
            if (i.second.dirty)
            {
                i.second.dirty = false;

                switch (i.second.type)
                {
                case MaterialProperty::Type::Texture:
                    this->UpdateUniformTexture(i.second.name, i.second.texture, instance_cmd_dirty);
                    break;
                case MaterialProperty::Type::VectorArray:
                    this->UpdateUniformMember(i.second.name, i.second.vector_array.Bytes(), i.second.vector_array.SizeInBytes(), instance_cmd_dirty);
                    break;
                case MaterialProperty::Type::MatrixArray:
                    this->UpdateUniformMember(i.second.name, i.second.matrix_array.Bytes(), i.second.matrix_array.SizeInBytes(), instance_cmd_dirty);
                    break;
                case MaterialProperty::Type::StorageBuffer:
                    this->UpdateStorageBuffer(i.second.name, i.second.buffer.lock(), instance_cmd_dirty);
                    break;
                case MaterialProperty::Type::UniformTexelBuffer:
                    this->UpdateUniformTexelBuffer(i.second.name, i.second.buffer.lock(), instance_cmd_dirty);
                    break;
                case MaterialProperty::Type::StorageTexelBuffer:
                    this->UpdateStorageTexelBuffer(i.second.name, i.second.buffer.lock(), instance_cmd_dirty);
                    break;
                default:
                    this->UpdateUniformMember(i.second.name, &i.second.data, i.second.size, instance_cmd_dirty);
                    break;
                }
            }
        }

        if (instance_cmd_dirty)
        {
            this->MarkInstanceCmdDirty();
        }
    }

    int Material::FindUniformSetIndex(const String& name)
    {
        for (int i = 0; i < m_uniform_sets.Size(); ++i)
        {
            Vector<VkDescriptorSetLayoutBinding> layout_bindings;

            for (int j = 0; j < m_uniform_sets[i].buffers.Size(); ++j)
            {
                const auto& buffer = m_uniform_sets[i].buffers[j];

                for (int k = 0; k < buffer.members.Size(); ++k)
                {
                    const auto& member = buffer.members[k];

                    if (member.name == name)
                    {
                        return i;
                    }
                }

                for (int j = 0; j < m_uniform_sets[i].textures.Size(); ++j)
                {
                    const auto& uniform_texture = m_uniform_sets[i].textures[j];

                    if (uniform_texture.name == name)
                    {
                        return i;
                    }
                }
            }
        }

        return -1;
    }

    void Material::UpdateUniformMember(const String& name, const void* data, int size, bool& instance_cmd_dirty)
    {
        for (int i = 0; i < m_uniform_sets.Size(); ++i)
        {
            for (int j = 0; j < m_uniform_sets[i].buffers.Size(); ++j)
            {
                auto& buffer = m_uniform_sets[i].buffers[j];

                for (int k = 0; k < buffer.members.Size(); ++k)
                {
                    const auto& member = buffer.members[k];

                    if (member.name == name && size <= member.size)
                    {
                        if (!buffer.buffer)
                        {
                            Display::Instance()->CreateUniformBuffer(m_descriptor_sets[i], buffer);
                            instance_cmd_dirty = true;
                        }
                        Display::Instance()->UpdateBuffer(buffer.buffer, member.offset, data, size);
                        return;
                    }
                }
            }
        }
    }

    void Material::UpdateUniformTexture(const String& name, const Ref<Texture>& texture, bool& instance_cmd_dirty)
    {
        for (int i = 0; i < m_uniform_sets.Size(); ++i)
        {
            for (int j = 0; j < m_uniform_sets[i].textures.Size(); ++j)
            {
                const auto& uniform_texture = m_uniform_sets[i].textures[j];

                if (uniform_texture.name == name)
                {
                    bool is_storage = (uniform_texture.stage == VK_SHADER_STAGE_COMPUTE_BIT);
                    Display::Instance()->UpdateUniformTexture(m_descriptor_sets[i], uniform_texture.binding, is_storage, texture);
                    instance_cmd_dirty = true;
                    return;
                }
            }
        }
    }

    void Material::UpdateStorageBuffer(const String& name, const Ref<BufferObject>& buffer, bool& instance_cmd_dirty)
    {
        for (int i = 0; i < m_uniform_sets.Size(); ++i)
        {
            for (int j = 0; j < m_uniform_sets[i].storage_buffers.Size(); ++j)
            {
                const auto& storage_buffer = m_uniform_sets[i].storage_buffers[j];

                if (storage_buffer.name == name)
                {
                    Display::Instance()->UpdateStorageBuffer(m_descriptor_sets[i], storage_buffer.binding, buffer);
                    instance_cmd_dirty = true;
                    return;
                }
            }
        }
    }

    void Material::UpdateUniformTexelBuffer(const String& name, const Ref<BufferObject>& buffer, bool& instance_cmd_dirty)
    {
        for (int i = 0; i < m_uniform_sets.Size(); ++i)
        {
            for (int j = 0; j < m_uniform_sets[i].uniform_texel_buffers.Size(); ++j)
            {
                const auto& texel_buffer = m_uniform_sets[i].uniform_texel_buffers[j];

                if (texel_buffer.name == name)
                {
                    Display::Instance()->UpdateTexelBuffer(m_descriptor_sets[i], texel_buffer.binding, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, buffer);
                    instance_cmd_dirty = true;
                    return;
                }
            }
        }
    }

    void Material::UpdateStorageTexelBuffer(const String& name, const Ref<BufferObject>& buffer, bool& instance_cmd_dirty)
    {
        for (int i = 0; i < m_uniform_sets.Size(); ++i)
        {
            for (int j = 0; j < m_uniform_sets[i].storage_texel_buffers.Size(); ++j)
            {
                const auto& texel_buffer = m_uniform_sets[i].storage_texel_buffers[j];

                if (texel_buffer.name == name)
                {
                    Display::Instance()->UpdateTexelBuffer(m_descriptor_sets[i], texel_buffer.binding, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, buffer);
                    instance_cmd_dirty = true;
                    return;
                }
            }
        }
    }
#elif VR_GLES
    void Material::ApplyUniforms() const
    {
        int texture_unit = 0;
        for (const auto& i : m_properties)
        {
            const MaterialProperty& p = i.second;
            switch (p.type)
            {
            case MaterialProperty::Type::Color:
                m_shader->SetUniform4f(p.name, 1, (const float*) &p.data.color);
                break;
            case MaterialProperty::Type::Vector:
                m_shader->SetUniform4f(p.name, 1, (const float*) &p.data.vector);
                break;
            case MaterialProperty::Type::Float:
                m_shader->SetUniform1f(p.name, p.data.float_value);
                break;
            case MaterialProperty::Type::Texture:
                if (p.texture)
                {
                    glActiveTexture(GL_TEXTURE0 + texture_unit);
                    p.texture->Bind();
                    m_shader->SetUniform1i(p.name, texture_unit);
                    texture_unit += 1;
                }
                break;
            case MaterialProperty::Type::Matrix:
                m_shader->SetUniformMatrix(p.name, 1, (const float*) &p.data.matrix);
                break;
            case MaterialProperty::Type::VectorArray:
                m_shader->SetUniform4f(p.name, p.vector_array.Size(), (const float*) &p.vector_array[0]);
                break;
            case MaterialProperty::Type::MatrixArray:
                m_shader->SetUniformMatrix(p.name, p.matrix_array.Size(), (const float*) &p.matrix_array[0]);
                break;
            case MaterialProperty::Type::Int:
                m_shader->SetUniform1i(p.name, p.data.int_value);
                break;
            }
        }
    }
#endif
}
