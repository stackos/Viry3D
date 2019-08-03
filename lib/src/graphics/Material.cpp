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

        this->SetTexture(MaterialProperty::TEXTURE, Texture::GetSharedWhiteTexture());
		this->SetVector(MaterialProperty::TEXTURE_SCALE_OFFSET, Vector4(1, 1, 0, 0));
		this->SetColor(MaterialProperty::COLOR, Color(1, 1, 1, 1));
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
					m_unifrom_buffers[i][j].uniform_buffer.clear();
                }
            }
        }
        m_unifrom_buffers.Clear();
        
        for (int i = 0; i < m_samplers.Size(); ++i)
        {
            if (m_samplers[i].sampler_group)
            {
                driver.destroySamplerGroup(m_samplers[i].sampler_group);
				m_samplers[i].sampler_group.clear();
            }
        }
        m_samplers.Clear();
    }
    
	const Ref<Shader>& Material::GetLightAddShader()
	{
		if (!m_light_add_shader)
		{
			auto& keywords = m_shader->GetKeywords();
			Vector<String> new_keywords;
			for (auto& i : keywords)
			{
				new_keywords.Add(i);
			}
			new_keywords.Add("LIGHT_ADD_ON");
			m_light_add_shader = Shader::Find(m_shader->GetName(), new_keywords, true);
		}

		return m_light_add_shader;
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
    
	void Material::EnableKeyword(const String& keyword)
	{
		auto& keywords = m_shader->GetKeywords();
		if (!keywords.Contains(keyword))
		{
			Vector<String> new_keywords;
			for (auto& i : keywords)
			{
				new_keywords.Add(i);
			}
			new_keywords.Add(keyword);
			m_shader = Shader::Find(m_shader->GetName(), new_keywords);
		}
	}

	void Material::DisableKeyword(const String& keyword)
	{
		auto& keywords = m_shader->GetKeywords();
		if (keywords.Contains(keyword))
		{
			Vector<String> new_keywords;
			for (auto& i : keywords)
			{
				new_keywords.Add(i);
			}
			new_keywords.Remove(keyword);
			m_shader = Shader::Find(m_shader->GetName(), new_keywords);
		}
	}

    void Material::Prepare(int pass)
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
        
        auto& driver = Engine::Instance()->GetDriverApi();
        
        for (int i = 0; i < m_unifrom_buffers.Size(); ++i)
        {
            auto& unifrom_buffers = m_unifrom_buffers[i];
            
            if (pass >= 0 && pass != i)
            {
                continue;
            }
            
            for (int j = 0; j < unifrom_buffers.Size(); ++j)
            {
                auto& unifrom_buffer = unifrom_buffers[j];
                
                if (unifrom_buffer.dirty)
                {
                    unifrom_buffer.dirty = false;
                    
                    void* buffer = Memory::Alloc<void>(unifrom_buffer.buffer.Size());
                    Memory::Copy(buffer, unifrom_buffer.buffer.Bytes(), unifrom_buffer.buffer.Size());
                    driver.loadUniformBuffer(unifrom_buffer.uniform_buffer, filament::backend::BufferDescriptor(buffer, unifrom_buffer.buffer.Size(), FreeBufferCallback));
                }
            }
        }
        
        for (int i = 0; i < m_samplers.Size(); ++i)
        {
            auto& sampler_group = m_samplers[i];
            
            if (pass >= 0 && pass != i)
            {
                continue;
            }
            
            if (sampler_group.dirty)
            {
                sampler_group.dirty = false;
                
                filament::backend::SamplerGroup samplers(sampler_group.samplers.Size());
                for (int j = 0; j < sampler_group.samplers.Size(); ++j)
                {
                    const auto& sampler = sampler_group.samplers[j];
					samplers.setSampler(j, sampler.texture->GetTexture(), sampler.texture->GetSampler());
                }
                driver.updateSamplerGroup(sampler_group.sampler_group, std::move(samplers));
            }
        }
    }
    
    void Material::UpdateUniformMember(const String& name, const void* data, int size)
    {
        auto& driver = Engine::Instance()->GetDriverApi();
        
        for (int i = 0; i < m_shader->GetPassCount(); ++i)
        {
            const auto& pass = m_shader->GetPass(i);
            
            for (int j = 0; j < pass.uniforms.Size(); ++j)
            {
                const auto& uniform = pass.uniforms[j];
				bool find = false;
                
                for (int k = 0; k < uniform.members.Size(); ++k)
                {
                    const auto& member = uniform.members[k];
                    
                    if (name == member.name)
                    {
                        auto& unifrom_buffer = m_unifrom_buffers[i][uniform.binding];
                        
                        if (!unifrom_buffer.uniform_buffer)
                        {
                            unifrom_buffer.uniform_buffer = driver.createUniformBuffer(uniform.size, filament::backend::BufferUsage::DYNAMIC);
                            unifrom_buffer.buffer = ByteBuffer(uniform.size);
                        }
                        
                        assert(size <= member.size);
                        
                        Memory::Copy(&unifrom_buffer.buffer[member.offset], data, size);
                        
                        unifrom_buffer.dirty = true;

						find = true;
						break;
                    }
                }

				if (find)
				{
					break;
				}
            }
        }
    }
    
    void Material::UpdateUniformTexture(const String& name, const Ref<Texture>& texture)
    {
        auto& driver = Engine::Instance()->GetDriverApi();
        
        for (int i = 0; i < m_shader->GetPassCount(); ++i)
        {
            const auto& pass = m_shader->GetPass(i);
            
            for (int j = 0; j < pass.samplers.Size(); ++j)
            {
                const auto& group = pass.samplers[j];
                
				if (group.binding == (int) Shader::BindingPoint::PerMaterialFragment)
				{
					for (int k = 0; k < group.samplers.Size(); ++k)
					{
						const auto& sampler = group.samplers[k];

						if (name == sampler.name)
						{
							auto& sampler_group = m_samplers[i];

							if (!sampler_group.sampler_group)
							{
								sampler_group.sampler_group = driver.createSamplerGroup(group.samplers.Size());
								sampler_group.samplers.Resize(group.samplers.Size());
							}

							sampler_group.samplers[k].binding = sampler.binding;
							sampler_group.samplers[k].texture = texture;

							sampler_group.dirty = true;
						}
					}

					break;
				}
            }
        }
    }
    
    void Material::SetScissor(int target_width, int target_height)
    {
		auto& driver = Engine::Instance()->GetDriverApi();
        
		// set scissor
		int32_t scissor_left = (int32_t) (m_scissor_rect.x * target_width);
		int32_t scissor_bottom = (int32_t) ((1.0f - (m_scissor_rect.y + m_scissor_rect.h)) * target_height);
		uint32_t scissor_width = (uint32_t) (m_scissor_rect.w * target_width);
		uint32_t scissor_height = (uint32_t) (m_scissor_rect.h * target_height);
		driver.setViewportScissor(scissor_left, scissor_bottom, scissor_width, scissor_height);
    }

	void Material::Bind(int pass)
	{
		auto& driver = Engine::Instance()->GetDriverApi();

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
			driver.bindSamplers((size_t) Shader::BindingPoint::PerMaterialFragment, samplers.sampler_group);
		}
	}
}
