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

#include "Material.h"
#include "Shader.h"
#include "Renderer.h"
#include "BufferObject.h"

namespace Viry3D
{
    Material::Material(const Ref<Shader>& shader):
        m_shader(shader)
    {
        m_shader->CreateDescriptorSets(m_descriptor_sets, m_uniform_sets);
    }

    Material::~Material()
    {
        this->Release();
    }

    void Material::Release()
    {
        VkDevice device = Display::Instance()->GetDevice();

        m_descriptor_sets.Clear();

        for (int i = 0; i < m_uniform_sets.Size(); ++i)
        {
            for (int j = 0; j < m_uniform_sets[i].buffers.Size(); j++)
            {
                if (m_uniform_sets[i].buffers[j].buffer)
                {
                    m_uniform_sets[i].buffers[j].buffer->Destroy(device);
                    m_uniform_sets[i].buffers[j].buffer.reset();
                }
            }
        }
        m_uniform_sets.Clear();
    }
    
    void Material::SetShader(const Ref<Shader>& shader)
    {
        this->Release();

        m_shader = shader;
        m_shader->CreateDescriptorSets(m_descriptor_sets, m_uniform_sets);

        this->MarkInstanceCmdDirty();
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

    void Material::SetTexture(const String& name, const Ref<Texture>& texture)
    {
        MaterialProperty* property_ptr;
        if (m_properties.TryGet(name, &property_ptr))
        {
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

    void Material::SetVectorArray(const String& name, const Vector<Vector4>& array)
    {
        MaterialProperty* property_ptr;
        if (m_properties.TryGet(name, &property_ptr))
        {
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

    void Material::UpdateUniformSets()
    {
        bool instance_cmd_dirty = false;

        for (auto& i : m_properties)
        {
            if (i.second.dirty)
            {
                i.second.dirty = false;

                if (i.second.type == MaterialProperty::Type::Texture)
                {
                    this->UpdateUniformTexture(i.second.name, i.second.texture, instance_cmd_dirty);
                }
                else if (i.second.type == MaterialProperty::Type::VectorArray)
                {
                    this->UpdateUniformMember(i.second.name, i.second.vector_array.Bytes(), i.second.vector_array.SizeInBytes(), instance_cmd_dirty);
                }
                else
                {
                    this->UpdateUniformMember(i.second.name, &i.second.data, i.second.size, instance_cmd_dirty);
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
                    Display::Instance()->UpdateUniformTexture(m_descriptor_sets[i], uniform_texture.binding, texture);
                    instance_cmd_dirty = true;
                    return;
                }
            }
        }
    }

    void Material::MarkRendererOrderDirty()
    {
        for (auto i : m_renderers)
        {
            i->MarkRendererOrderDirty();
        }
    }

    void Material::MarkInstanceCmdDirty()
    {
        for (auto i : m_renderers)
        {
            i->MarkInstanceCmdDirty();
        }
    }
}
