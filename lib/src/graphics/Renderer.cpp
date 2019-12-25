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
#include "Engine.h"
#include "GameObject.h"
#include "Selection.h"

namespace Viry3D
{
    List<Renderer*> Renderer::m_renderers;

	void Renderer::PrepareAll()
	{
		for (auto i : m_renderers)
		{
            if (i->GetGameObject()->IsActiveInTree() && i->IsEnable())
            {
                i->Prepare();
            }
		}
	}

    Renderer::Renderer():
		m_cast_shadow(false),
		m_recieve_shadow(false),
        m_lightmap_scale_offset(1, 1, 0, 0),
        m_lightmap_index(-1)
    {
        m_renderers.AddLast(this);
    }
    
    Renderer::~Renderer()
    {
		auto& driver = Engine::Instance()->GetDriverApi();

		if (m_transform_uniform_buffer)
		{
			driver.destroyUniformBuffer(m_transform_uniform_buffer);
			m_transform_uniform_buffer.clear();
		}

        m_renderers.Remove(this);
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
        m_materials = materials;

        this->UpdateShaderKeywords();
    }

	void Renderer::EnableCastShadow(bool enable)
	{
		m_cast_shadow = enable;
	}
    
	void Renderer::EnableRecieveShadow(bool enable)
	{
		m_recieve_shadow = enable;
	}

    void Renderer::SetLightmapIndex(int index)
    {
        m_lightmap_index = index;
    }
    
    void Renderer::SetLightmapScaleOffset(const Vector4& vec)
    {
        m_lightmap_scale_offset = vec;
    }

    void Renderer::SetShaderKeywords(const Vector<String>& keywords)
    {
        m_shader_keywords = keywords;

        this->UpdateShaderKeywords();
    }

    void Renderer::EnableShaderKeyword(const String& keyword)
    {
        if (!m_shader_keywords.Contains(keyword))
        {
            m_shader_keywords.Add(keyword);

            this->UpdateShaderKeywords();
        }
    }

    const String* Renderer::GetShaderKey(int material_index)
    {
        return &m_shader_keys[material_index];
    }

    const String* Renderer::GetShaderKeyWithLightAddOn(int material_index)
    {
        return &m_shader_keys_light_add[material_index];
    }

    void Renderer::UpdateShaderKeywords()
    {
        auto shader_keywords_light_add = m_shader_keywords;
        shader_keywords_light_add.Add("LIGHT_ADD_ON");

        m_shader_keys.Resize(m_materials.Size());
        m_shader_keys_light_add.Resize(m_materials.Size());

        for (int i = 0; i < m_materials.Size(); ++i)
        {
            if (m_materials[i] && m_materials[i]->GetShaderName().Size() > 0)
            {
                m_shader_keys[i] = m_materials[i]->EnableKeywords(m_shader_keywords);
                m_shader_keys_light_add[i] = m_materials[i]->EnableKeywords(shader_keywords_light_add);
            }
        }
    }
    
    Vector<filament::backend::RenderPrimitiveHandle> Renderer::GetPrimitives()
    {
        return Vector<filament::backend::RenderPrimitiveHandle>();
    }

	void Renderer::Prepare()
	{
		auto& driver = Engine::Instance()->GetDriverApi();
		const auto& materials = this->GetMaterials();

		for (int i = 0; i < materials.Size(); ++i)
		{
			auto& material = materials[i];
			if (material)
			{
				material->Prepare();
			}
		}

		if (!m_transform_uniform_buffer)
		{
			m_transform_uniform_buffer = driver.createUniformBuffer(sizeof(RendererUniforms), filament::backend::BufferUsage::DYNAMIC);
		}

        Bounds bounds = this->GetLocalBounds();
        Vector3 bounds_position = bounds.GetCenter();
        Vector3 bounds_size = bounds.GetSize();

		RendererUniforms renderer_uniforms;
		renderer_uniforms.model_matrix = this->GetTransform()->GetLocalToWorldMatrix();
        renderer_uniforms.bounds_matrix = Matrix4x4::TRS(bounds_position, Quaternion::Identity(), bounds_size);
        renderer_uniforms.bounds_color = (Selection::GetGameObject() == this->GetGameObject()) ? Color(1, 0, 0, 1) : Color(0, 1, 0, 1);
		renderer_uniforms.lightmap_scale_offset = m_lightmap_scale_offset;
		renderer_uniforms.lightmap_index = Vector4((float) m_lightmap_index);

		void* buffer = driver.allocate(sizeof(RendererUniforms));
		Memory::Copy(buffer, &renderer_uniforms, sizeof(RendererUniforms));
		driver.loadUniformBuffer(m_transform_uniform_buffer, filament::backend::BufferDescriptor(buffer, sizeof(RendererUniforms)));
	}
}
