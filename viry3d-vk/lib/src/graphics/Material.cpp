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
        VkDevice device = Display::GetDisplay()->GetDevice();

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

        for (auto i : m_renderers)
        {
            i->MarkRendererOrderDirty();
        }
    }

    void Material::OnSetRenderer(Renderer* renderer)
    {
        m_renderers.AddLast(renderer);
    }

    void Material::OnUnSetRenderer(Renderer* renderer)
    {
        m_renderers.Remove(renderer);
    }

    void Material::SetMatrix(const String& name, const Matrix4x4& mat)
    {
        this->SetProperty(name, mat, Property::Type::Matrix);
    }

    void Material::UpdateUniformSets()
    {
        for (auto i : m_properties)
        {
            if (i.second.dirty)
            {
                i.second.dirty = false;
                this->UpdateUniformMember(i.second.name, &i.second.data, i.second.size);
            }
        }
    }

    void Material::UpdateUniformMember(const String& name, const void* data, int size)
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

                    if (member.name == name && size <= member.size)
                    {
                        Display::GetDisplay()->UpdateBuffer(buffer.buffer, member.offset, data, size);
                        return;
                    }
                }
            }
        }
    }
}
