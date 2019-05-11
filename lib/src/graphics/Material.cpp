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
#include "Engine.h"
#include "Camera.h"

namespace Viry3D
{
    Material::Material(const Ref<Shader>& shader):
        m_shader(shader),
        m_scissor_rect(0, 0, 1, 1)
    {
        m_unifrom_buffers.Resize(shader->GetPassCount());
        for (int i = 0; i < m_unifrom_buffers.Size(); ++i)
        {
            m_unifrom_buffers[i].Resize((int) Shader::BindingPoint::Count);
        }
        
        m_samplers.Resize(shader->GetPassCount());
    }
    
    Material::~Material()
    {
        auto& driver = Engine::Instance()->GetDriverApi();
        
        for (int i = 0; i < m_unifrom_buffers.Size(); ++i)
        {
            for (int j = 0; j < m_unifrom_buffers[i].Size(); ++j)
            {
                if (m_unifrom_buffers[i][j].uniform_buffer)
                {
                    driver.destroyUniformBuffer(m_unifrom_buffers[i][j].uniform_buffer);
                }
            }
        }
        m_unifrom_buffers.Clear();
        
        for (int i = 0; i < m_samplers.Size(); ++i)
        {
            if (m_samplers[i].sampler_group)
            {
                driver.destroySamplerGroup(m_samplers[i].sampler_group);
            }
        }
        m_samplers.Clear();
    }
    
    int Material::GetQueue() const
    {
        if (m_queue)
        {
            return *m_queue;
        }
        
        return m_shader->GetQueue();
    }
    
    void Material::SetQueue(int queue)
    {
        m_queue = RefMake<int>(queue);
    }
    
    const Matrix4x4* Material::GetMatrix(const String& name) const
    {
        return this->GetProperty<Matrix4x4>(name, MaterialProperty::Type::Matrix);
    }
    
    void Material::SetMatrix(const String& name, const Matrix4x4& value)
    {
        this->SetProperty(name, value, MaterialProperty::Type::Matrix);
    }
    
    const Vector4* Material::GetVector(const String& name) const
    {
        return this->GetProperty<Vector4>(name, MaterialProperty::Type::Vector);
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
    
    void Material::SetScissorRect(const Rect& rect)
    {
        m_scissor_rect = rect;
    }
    
    void Material::Prepare()
    {
        for (auto& i : m_properties)
        {
            if (i.second.dirty)
            {
                i.second.dirty = false;
                
                switch (i.second.type)
                {
                    case MaterialProperty::Type::Texture:
                        this->UpdateUniformTexture(i.first, i.second.texture);
                        break;
                    case MaterialProperty::Type::VectorArray:
                        this->UpdateUniformMember(i.first, i.second.vector_array.Bytes(), i.second.vector_array.SizeInBytes());
                        break;
                    case MaterialProperty::Type::MatrixArray:
                        this->UpdateUniformMember(i.first, i.second.matrix_array.Bytes(), i.second.matrix_array.SizeInBytes());
                        break;
                    default:
                        this->UpdateUniformMember(i.first, &i.second.data, i.second.size);
                        break;
                }
            }
        }
        
        
    }
    
    void Material::UpdateUniformMember(const String& name, const void* data, int size)
    {
        
    }
    
    void Material::UpdateUniformTexture(const String& name, const Ref<Texture>& texture)
    {
        
    }
    
    void Material::Apply(const Camera* camera, int pass)
    {
        auto& driver = Engine::Instance()->GetDriverApi();
        
        // set scissor
        int target_width = camera->GetTargetWidth();
        int target_height = camera->GetTargetHeight();
        int32_t scissor_left = (int32_t) (m_scissor_rect.x * target_width);
        int32_t scissor_bottom = (int32_t) ((1.0f - (m_scissor_rect.y + m_scissor_rect.h)) * target_height);
        uint32_t scissor_width = (uint32_t) (m_scissor_rect.w * target_width);
        uint32_t scissor_height = (uint32_t) (m_scissor_rect.h * target_height);
        driver.setViewportScissor(scissor_left, scissor_bottom, scissor_width, scissor_height);
        
        const auto& unifrom_buffers = m_unifrom_buffers[pass];
        const auto& samplers = m_samplers[pass];
        
        // bind uniforms
        for (int i = 0; i < unifrom_buffers.Size(); ++i)
        {
            if (unifrom_buffers[i].uniform_buffer)
            {
                driver.bindUniformBuffer((size_t) i, unifrom_buffers[i].uniform_buffer);
            }
        }
        
        // bind samplers
        if (samplers.sampler_group)
        {
            driver.bindSamplers((size_t) Shader::BindingPoint::PerMaterialInstance, samplers.sampler_group);
        }
    }
}
