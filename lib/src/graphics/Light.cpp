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

#include "Light.h"
#include "Engine.h"
#include "Material.h"
#include "GameObject.h"
#include "Renderer.h"

namespace Viry3D
{
	List<Light*> Light::m_lights;
	Color Light::m_ambient_color(0, 0, 0, 0);

	void Light::SetAmbientColor(const Color& color)
	{
		m_ambient_color = color;
		for (auto i : m_lights)
		{
			i->m_dirty = true;
		}
	}

	void Light::RenderShadowMaps()
	{
		for (auto i : m_lights)
		{
			if (i->IsShadowEnable())
			{
				List<Renderer*> renderers;
				i->CullRenderers(Renderer::GetRenderers(), renderers);
			}
		}
	}

	void Light::CullRenderers(const List<Renderer*>& renderers, List<Renderer*>& result)
	{
		for (auto i : renderers)
		{
			int layer = i->GetGameObject()->GetLayer();
			if (i->IsCastShadow() && ((1 << layer) & m_culling_mask))
			{
				result.AddLast(i);
			}
		}
		result.Sort([](Renderer* a, Renderer* b) {
			const auto& materials_a = a->GetMaterials();
			int queue_a = 0;
			for (int i = 0; i < materials_a.Size(); ++i)
			{
				int queue = materials_a[i]->GetQueue();
				if (queue_a < queue)
				{
					queue_a = queue;
				}
			}
			const auto& materials_b = b->GetMaterials();
			int queue_b = 0;
			for (int i = 0; i < materials_b.Size(); ++i)
			{
				int queue = materials_b[i]->GetQueue();
				if (queue_b < queue)
				{
					queue_b = queue;
				}
			}
			return queue_a < queue_b;
		});
	}

    Light::Light():
		m_dirty(true),
        m_type(LightType::Directional),
		m_color(1, 1, 1, 1),
		m_intensity(1.0f),
		m_range(1.0f),
		m_spot_angle(30.0f),
		m_shadow_enable(false),
		m_culling_mask(0xffffffff)
    {
		m_lights.AddLast(this);
    }

	Light::~Light()
    {
		auto& driver = Engine::Instance()->GetDriverApi();

		if (m_light_uniform_buffer)
		{
			driver.destroyUniformBuffer(m_light_uniform_buffer);
			m_light_uniform_buffer.clear();
		}

		m_lights.Remove(this);
    }

	void Light::OnTransformDirty()
	{
		m_dirty = true;
	}

	void Light::SetType(LightType type)
	{
		m_type = type;
		m_dirty = true;
	}

	void Light::SetColor(const Color& color)
	{
		m_color = color;
		m_dirty = true;
	}

	void Light::SetIntensity(float intensity)
	{
		m_intensity = intensity;
		m_dirty = true;
	}

	void Light::SetRange(float range)
	{
		m_range = range;
		m_dirty = true;
	}

	void Light::SetSpotAngle(float angle)
	{
		m_spot_angle = angle;
		m_dirty = true;
	}

	void Light::EnableShadow(bool enable)
	{
		m_shadow_enable = enable;
	}

	void Light::SetCullingMask(uint32_t mask)
	{
		m_culling_mask = mask;
	}

	void Light::Prepare()
	{
		if (!m_dirty)
		{
			return;
		}
		m_dirty = false;

		auto& driver = Engine::Instance()->GetDriverApi();
		if (!m_light_uniform_buffer)
		{
			m_light_uniform_buffer = driver.createUniformBuffer(sizeof(LightUniforms), filament::backend::BufferUsage::DYNAMIC);
		}

		LightUniforms light_uniforms;
		light_uniforms.ambient_color = this->GetAmbientColor();
		if (this->GetType() == LightType::Directional)
		{
			light_uniforms.light_pos = -this->GetTransform()->GetForward();
			light_uniforms.light_pos.w = 0.0f;
		}
		else
		{
			light_uniforms.light_pos = this->GetTransform()->GetPosition();
			light_uniforms.light_pos.w = 1.0f;
		}
		light_uniforms.light_color = this->GetColor() * this->GetIntensity();
		light_uniforms.light_color.a = (float) this->GetType();
		light_uniforms.light_atten = Vector4(0, 0, 0, 0);
		if (this->GetType() == LightType::Spot || this->GetType() == LightType::Point)
		{
			light_uniforms.light_atten.z = 1.0f / (this->GetRange() * this->GetRange());
		}
		if (this->GetType() == LightType::Spot)
		{
			light_uniforms.light_atten.x = cos(this->GetSpotAngle() / 2 * Mathf::Deg2Rad);
			light_uniforms.light_atten.y = 1.0f / (light_uniforms.light_atten.x - cos(this->GetSpotAngle() / 4 * Mathf::Deg2Rad));
			light_uniforms.spot_light_dir = -this->GetTransform()->GetForward();
		}

		void* buffer = driver.allocate(sizeof(LightUniforms));
		Memory::Copy(buffer, &light_uniforms, sizeof(LightUniforms));
		driver.loadUniformBuffer(m_light_uniform_buffer, filament::backend::BufferDescriptor(buffer, sizeof(LightUniforms)));
	}
}
