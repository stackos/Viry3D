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
#include "SkinnedMeshRenderer.h"
#include "Texture.h"

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
			if ((i->GetType() == LightType::Directional || i->GetType() == LightType::Spot) &&
				i->IsShadowEnable())
			{
				List<Renderer*> renderers;
				i->CullRenderers(Renderer::GetRenderers(), renderers);
				i->UpdateViewUniforms();
				i->Draw(renderers);
			}
		}
	}

	void Light::CullRenderers(const List<Renderer*>& renderers, List<Renderer*>& result)
	{
		for (auto i : renderers)
		{
			int layer = i->GetGameObject()->GetLayer();
			if (((1 << layer) & m_culling_mask) && i->IsCastShadow())
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

	void Light::UpdateViewUniforms()
	{
		auto& driver = Engine::Instance()->GetDriverApi();
		if (!m_view_uniform_buffer)
		{
			m_view_uniform_buffer = driver.createUniformBuffer(sizeof(ViewUniforms), filament::backend::BufferUsage::DYNAMIC);
		}

		ViewUniforms view_uniforms;
		view_uniforms.view_matrix = this->GetViewMatrix();
		view_uniforms.projection_matrix = this->GetProjectionMatrix();
		view_uniforms.camera_pos = this->GetTransform()->GetPosition();

		void* buffer = driver.allocate(sizeof(ViewUniforms));
		Memory::Copy(buffer, &view_uniforms, sizeof(ViewUniforms));
		driver.loadUniformBuffer(m_view_uniform_buffer, filament::backend::BufferDescriptor(buffer, sizeof(ViewUniforms)));
	}

	void Light::Draw(const List<Renderer*>& renderers)
	{
		auto& driver = Engine::Instance()->GetDriverApi();

		int target_width = m_shadow_texture_size;
		int target_height = m_shadow_texture_size;

		filament::backend::RenderTargetHandle target;
		filament::backend::RenderPassParams params;
		params.flags.clear = filament::backend::TargetBufferFlags::NONE;
		params.flags.discardStart = filament::backend::TargetBufferFlags::NONE;
		params.flags.discardEnd = filament::backend::TargetBufferFlags::NONE;

		if (!m_render_target)
		{
			filament::backend::TargetBufferFlags target_flags = filament::backend::TargetBufferFlags::NONE;
			filament::backend::TargetBufferInfo color = { };
			filament::backend::TargetBufferInfo depth = { };
			filament::backend::TargetBufferInfo stencil = { };

			target_flags |= filament::backend::TargetBufferFlags::DEPTH;
			depth.handle = m_shadow_texture->GetTexture();

			m_render_target = driver.createRenderTarget(
				target_flags,
				target_width,
				target_height,
				1,
				color,
				depth,
				stencil);
		}
		target = m_render_target;

		params.flags.clear = filament::backend::TargetBufferFlags::DEPTH;
		params.flags.discardStart |= filament::backend::TargetBufferFlags::COLOR;

		params.viewport.left = 0;
		params.viewport.bottom = 0;
		params.viewport.width = (uint32_t) target_width;
		params.viewport.height = (uint32_t) target_height;

		driver.beginRenderPass(target, params);

		driver.bindUniformBuffer((size_t) Shader::BindingPoint::PerView, m_view_uniform_buffer);

		for (auto i : renderers)
		{
			this->DrawRenderer(i);
		}

		driver.endRenderPass();

		driver.flush();
	}

	void Light::DrawRenderer(Renderer* renderer)
	{
		auto& driver = Engine::Instance()->GetDriverApi();

		driver.bindUniformBuffer((size_t) Shader::BindingPoint::PerRenderer, renderer->GetTransformUniformBuffer());

		SkinnedMeshRenderer* skin = dynamic_cast<SkinnedMeshRenderer*>(renderer);
		if (skin && skin->GetBonesUniformBuffer())
		{
			driver.bindUniformBuffer((size_t) Shader::BindingPoint::PerRendererBones, skin->GetBonesUniformBuffer());
		}

		const auto& materials = renderer->GetMaterials();
		for (int i = 0; i < materials.Size(); ++i)
		{
			auto& material = materials[i];
			if (material)
			{
				filament::backend::RenderPrimitiveHandle primitive;

				auto primitives = renderer->GetPrimitives();
				if (i < primitives.Size())
				{
					primitive = primitives[i];
				}
				else
				{
					break;
				}

				if (primitive)
				{
					const auto& shader = material->GetShader();

					material->SetScissor(m_shadow_texture_size, m_shadow_texture_size);

					for (int j = 0; j < shader->GetPassCount(); ++j)
					{
						if (shader->GetPass(j).queue <= (int) Shader::Queue::AlphaTest &&
							shader->GetPass(j).pipeline.rasterState.depthWrite)
						{
							material->Bind(j);

							Ref<Shader> shadow_shader;
							if (skin && skin->GetBonePaths().Size() > 0)
							{
								shadow_shader = Shader::Find("ShadowMap", { "SKIN_ON" });
							}
							else
							{
								shadow_shader = Shader::Find("ShadowMap");
							}

							const auto& pipeline = shadow_shader->GetPass(0).pipeline;
							driver.draw(pipeline, primitive);
						}
					}
				}
			}
		}
	}

    Light::Light():
		m_dirty(true),
        m_type(LightType::Directional),
		m_color(1, 1, 1, 1),
		m_intensity(1.0f),
		m_range(1.0f),
		m_spot_angle(30.0f),
		m_shadow_enable(false),
		m_shadow_texture_size(0),
		m_shadow_strength(1.0f),
		m_shadow_z_bias(0.0001f),
		m_shadow_slope_bias(0.0001f),
		m_near_clip(0.3f),
		m_far_clip(1000),
		m_orthographic_size(1),
		m_view_matrix_dirty(true),
		m_projection_matrix_dirty(true),
		m_culling_mask(0xffffffff)
    {
		m_lights.AddLast(this);

		this->SetShadowTextureSize(1024);
    }

	Light::~Light()
    {
		auto& driver = Engine::Instance()->GetDriverApi();

		if (m_view_uniform_buffer)
		{
			driver.destroyUniformBuffer(m_view_uniform_buffer);
			m_view_uniform_buffer.clear();
		}

		if (m_light_uniform_buffer)
		{
			driver.destroyUniformBuffer(m_light_uniform_buffer);
			m_light_uniform_buffer.clear();
		}

		if (m_sampler_group)
		{
			driver.destroySamplerGroup(m_sampler_group);
			m_sampler_group.clear();
		}

		if (m_render_target)
		{
			driver.destroyRenderTarget(m_render_target);
			m_render_target.clear();
		}

		m_lights.Remove(this);
    }

	void Light::OnTransformDirty()
	{
		m_dirty = true;
		m_view_matrix_dirty = true;
	}

	void Light::SetType(LightType type)
	{
		m_type = type;
		m_dirty = true;
		m_projection_matrix_dirty = true;
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
		m_projection_matrix_dirty = true;
	}

	void Light::EnableShadow(bool enable)
	{
		m_shadow_enable = enable;
	}

	void Light::SetShadowTextureSize(int size)
	{
		auto& driver = Engine::Instance()->GetDriverApi();

		if (m_shadow_texture_size != size)
		{
			m_shadow_texture_size = size;
			m_shadow_texture = Texture::CreateRenderTexture(
				m_shadow_texture_size,
				m_shadow_texture_size,
				Texture::SelectDepthFormat(),
				FilterMode::Linear,
				SamplerAddressMode::ClampToEdge);

			if (!m_sampler_group)
			{
				m_sampler_group = driver.createSamplerGroup(1);
			}

			filament::backend::SamplerGroup samplers(1);
			samplers.setSampler(0, m_shadow_texture->GetTexture(), m_shadow_texture->GetSampler());
			driver.updateSamplerGroup(m_sampler_group, std::move(samplers));

			if (m_render_target)
			{
				driver.destroyRenderTarget(m_render_target);
				m_render_target.clear();
			}
		}
	}

	void Light::SetShadowStrength(float strength)
	{
		m_shadow_strength = strength;
		m_dirty = true;
	}

	void Light::SetShadowZBias(float bias)
	{
		m_shadow_z_bias = bias;
		m_dirty = true;
	}

	void Light::SetShadowSlopeBias(float bias)
	{
		m_shadow_slope_bias = bias;
		m_dirty = true;
	}

	void Light::SetNearClip(float clip)
	{
		m_near_clip = clip;
		m_projection_matrix_dirty = true;
	}

	void Light::SetFarClip(float clip)
	{
		m_far_clip = clip;
		m_projection_matrix_dirty = true;
	}

	void Light::SetOrthographicSize(float size)
	{
		m_orthographic_size = size;
		m_projection_matrix_dirty = true;
	}

	const Matrix4x4& Light::GetViewMatrix()
	{
		if (m_view_matrix_dirty)
		{
			m_view_matrix_dirty = false;

			m_view_matrix = Matrix4x4::LookTo(this->GetTransform()->GetPosition(), this->GetTransform()->GetForward(), this->GetTransform()->GetUp());
		}

		return m_view_matrix;
	}

	const Matrix4x4& Light::GetProjectionMatrix()
	{
		if (m_projection_matrix_dirty)
		{
			m_projection_matrix_dirty = false;

			float view_width = (float) m_shadow_texture_size;
			float view_height = (float) m_shadow_texture_size;

			if (this->GetType() == LightType::Directional)
			{
				float ortho_size = m_orthographic_size;
				float top = ortho_size;
				float bottom = -ortho_size;
				float plane_h = ortho_size * 2;
				float plane_w = plane_h * view_width / view_height;
				m_projection_matrix = Matrix4x4::Ortho(-plane_w / 2, plane_w / 2, bottom, top, m_near_clip, m_far_clip);
			}
			else
			{
				m_projection_matrix = Matrix4x4::Perspective(m_spot_angle, view_width / view_height, m_near_clip, m_far_clip);
			}
		}

		return m_projection_matrix;
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
			m_light_uniform_buffer = driver.createUniformBuffer(sizeof(LightFragmentUniforms), filament::backend::BufferUsage::DYNAMIC);
		}

		LightFragmentUniforms light_uniforms;
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
		light_uniforms.shadow_params = Vector4(m_shadow_strength, m_shadow_z_bias, m_shadow_slope_bias, 1.0f / m_shadow_texture_size * 3);

		void* buffer = driver.allocate(sizeof(LightFragmentUniforms));
		Memory::Copy(buffer, &light_uniforms, sizeof(LightFragmentUniforms));
		driver.loadUniformBuffer(m_light_uniform_buffer, filament::backend::BufferDescriptor(buffer, sizeof(LightFragmentUniforms)));
	}
}
