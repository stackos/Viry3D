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
        VkDevice device = Display::Instance()->GetDevice();

        for (int i = 0; i < m_uniform_sets.Size(); ++i)
        {
            for (int j = 0; j < m_uniform_sets[i].buffers.Size(); j++)
            {
                m_uniform_sets[i].buffers[j].buffer->Destroy(device);
                m_uniform_sets[i].buffers[j].buffer.reset();
            }
        }
        m_uniform_sets.Clear();
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

    bool Material::HasProperty(const String& name) const
    {
        return m_properties.Contains(name);
    }

    void Material::SetMatrix(const String& name, const Matrix4x4& value, int dynamic_index)
    {
        this->SetProperty(name, value, MaterialProperty::Type::Matrix, dynamic_index);
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
        Vector<MaterialProperty>* properties_ptr;
        if (m_properties.TryGet(name, &properties_ptr))
        {
            (*properties_ptr)[0].texture = texture;
            (*properties_ptr)[0].dirty = true;
        }
        else
        {
            Vector<MaterialProperty> properties;

            MaterialProperty property;
            property.name = name;
            property.type = MaterialProperty::Type::Texture;
            property.texture = texture;
            property.dirty = true;
            property.dynamic_index = 0;

            properties.Add(property);
            m_properties.Add(name, properties);
        }
    }

    void Material::UpdateUniformSets()
    {
        bool instance_cmd_dirty = false;

        for (auto& i : m_properties)
        {
            for (int j = 0; j < i.second.Size(); ++j)
            {
                MaterialProperty& property = i.second[j];
                if (property.dirty)
                {
                    property.dirty = false;

                    if (property.type == MaterialProperty::Type::Texture)
                    {
                        this->UpdateUniformTexture(property.name, property.texture);

                        instance_cmd_dirty = true;
                    }
                    else
                    {
                        this->UpdateUniformMember(property.name, &property.data, property.size, property.dynamic_index);
                    }
                }
            }
        }

        if (instance_cmd_dirty)
        {
            this->MarkInstanceCmdDirty();
        }
    }

    int Material::FindUniformSetIndex(const String& name) const
    {
        for (int i = 0; i < m_uniform_sets.Size(); ++i)
        {
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

    void Material::UpdateUniformMember(const String& name, const void* data, int size, int dynamic_index)
    {
        for (int i = 0; i < m_uniform_sets.Size(); ++i)
        {
            for (int j = 0; j < m_uniform_sets[i].buffers.Size(); ++j)
            {
                const auto& buffer = m_uniform_sets[i].buffers[j];

                for (int k = 0; k < buffer.members.Size(); ++k)
                {
                    const auto& member = buffer.members[k];

                    if (member.name == name && size <= member.size)
                    {
                        Display::Instance()->UpdateBuffer(buffer.buffer, buffer.offset_alignment * dynamic_index + member.offset, data, size);
                        return;
                    }
                }
            }
        }
    }

    void Material::UpdateUniformTexture(const String& name, const Ref<Texture>& texture)
    {
        for (int i = 0; i < m_uniform_sets.Size(); ++i)
        {
            for (int j = 0; j < m_uniform_sets[i].textures.Size(); ++j)
            {
                const auto& uniform_texture = m_uniform_sets[i].textures[j];

                if (uniform_texture.name == name)
                {
                    Display::Instance()->UpdateUniformTexture(m_descriptor_sets[i], uniform_texture.binding, texture);
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
