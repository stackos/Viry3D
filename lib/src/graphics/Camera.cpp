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

#include "Camera.h"
#include "GameObject.h"
#include "Texture.h"
#include "RenderTarget.h"
#include "Engine.h"
#include "Renderer.h"
#include "Material.h"
#include "SkinnedMeshRenderer.h"
#include "Light.h"
#include "time/Time.h"
#include "postprocessing/PostProcessing.h"

namespace Viry3D
{
	List<Camera*> Camera::m_cameras;
	Camera* Camera::m_current_camera = nullptr;
	bool Camera::m_cameras_order_dirty = false;
	Ref<Mesh> Camera::m_quad_mesh;
	Ref<Material> Camera::m_blit_material;

	void Camera::Init()
	{
	
	}

	void Camera::Done()
	{
		m_quad_mesh.reset();
		m_blit_material.reset();
	}

	void Camera::RenderAll()
	{
		const auto& lights = Light::GetLights();
		for (auto i : lights)
		{
			i->Prepare();
		}

		if (m_cameras_order_dirty)
		{
			m_cameras_order_dirty = false;
			m_cameras.Sort([](Camera* a, Camera* b) {
				return a->GetDepth() < b->GetDepth();
			});
		}

		for (auto i : m_cameras)
		{
			if (i->GetGameObject()->IsActiveInTree())
			{
				m_current_camera = i;

				List<Renderer*> renderers;
				i->CullRenderers(Renderer::GetRenderers(), renderers);
				i->UpdateViewUniforms();
				i->Draw(renderers);
				i->PostProcessing();

				m_current_camera = nullptr;
			}
		}
	}
    
    void Camera::OnResizeAll(int width, int height)
    {
        for (auto i : m_cameras)
        {
            i->OnResize(width, height);
        }

		auto& renderers = Renderer::GetRenderers();
		for (auto i : renderers)
		{
			i->OnResize(width, height);
		}
    }
    
    void Camera::OnResize(int width, int height)
    {
        m_projection_matrix_dirty = true;
    }

    void Camera::CullRenderers(const List<Renderer*>& renderers, List<Renderer*>& result)
    {
        for (auto i : renderers)
        {
            int layer = i->GetGameObject()->GetLayer();
            if (i->GetGameObject()->IsActiveInTree() && ((1 << layer) & m_culling_mask) != 0)
            {
                result.AddLast(i);
            }
        }
        result.Sort([](Renderer* a, Renderer* b) {
            const auto& materials_a = a->GetMaterials();
            int queue_a = 0;
            for (int i = 0; i < materials_a.Size(); ++i)
            {
				if (materials_a[i])
				{
					int queue = materials_a[i]->GetQueue();
					if (queue_a < queue)
					{
						queue_a = queue;
					}
				}
            }
            const auto& materials_b = b->GetMaterials();
            int queue_b = 0;
            for (int i = 0; i < materials_b.Size(); ++i)
            {
				if (materials_b[i])
				{
					int queue = materials_b[i]->GetQueue();
					if (queue_b < queue)
					{
						queue_b = queue;
					}
				}
            }
            return queue_a < queue_b;
        });
    }

	void Camera::UpdateViewUniforms()
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
		float t = Time::GetTime();
		view_uniforms.time = Vector4(t / 20, t, t * 2, t * 3);

		// map depth range -1 ~ 1 to 0 ~ 1 for d3d
		if (Engine::Instance()->GetBackend() == filament::backend::Backend::D3D11)
		{
			Matrix4x4 depth_map_01 = {
				1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 0.5f, 0.5f,
				0, 0, 0, 1,
			};
			view_uniforms.projection_matrix = depth_map_01 * this->GetProjectionMatrix();
		}

		void* buffer = driver.allocate(sizeof(ViewUniforms));
		Memory::Copy(buffer, &view_uniforms, sizeof(ViewUniforms));
		driver.loadUniformBuffer(m_view_uniform_buffer, filament::backend::BufferDescriptor(buffer, sizeof(ViewUniforms)));
	}

	void Camera::Draw(const List<Renderer*>& renderers)
	{
		auto& driver = Engine::Instance()->GetDriverApi();

		int target_width = this->GetTargetWidth();
		int target_height = this->GetTargetHeight();
		bool has_post_processing = this->HasPostProcessing();

		filament::backend::RenderTargetHandle target;
		filament::backend::RenderPassParams params;
		params.flags.clear = filament::backend::TargetBufferFlags::NONE;
		params.flags.discardStart = filament::backend::TargetBufferFlags::NONE;
		params.flags.discardEnd = filament::backend::TargetBufferFlags::NONE;

		if (m_render_target_color || m_render_target_depth)
		{
			if (!m_render_target)
			{
				filament::backend::TargetBufferFlags target_flags = filament::backend::TargetBufferFlags::NONE;
				filament::backend::TargetBufferInfo color = { };
				filament::backend::TargetBufferInfo depth = { };
				filament::backend::TargetBufferInfo stencil = { };

				if (m_render_target_color)
				{
					target_flags |= filament::backend::TargetBufferFlags::COLOR;
					color.handle = m_render_target_color->GetTexture();
				}
				if (m_render_target_depth)
				{
					target_flags |= filament::backend::TargetBufferFlags::DEPTH;
					depth.handle = m_render_target_depth->GetTexture();
				}

				m_render_target = driver.createRenderTarget(
					target_flags,
					target_width,
					target_height,
					1,
					color,
					depth,
					stencil);
			}

			if (has_post_processing)
			{
				assert(m_render_target_color);

				filament::backend::TargetBufferFlags target_flags = filament::backend::TargetBufferFlags::NONE;
				TextureFormat color_format = TextureFormat::None;
				TextureFormat depth_format = TextureFormat::None;

				if (m_render_target_color)
				{
					target_flags |= filament::backend::TargetBufferFlags::COLOR;
					color_format = m_render_target_color->GetFormat();
				}
				if (m_render_target_depth)
				{
					target_flags |= filament::backend::TargetBufferFlags::DEPTH;
					depth_format = m_render_target_depth->GetFormat();
				}

				m_post_processing_target = RenderTarget::GetTemporaryRenderTarget(
					target_width,
					target_height,
					color_format,
					depth_format,
					FilterMode::Linear,
					SamplerAddressMode::ClampToEdge,
					target_flags);
				target = m_post_processing_target->target;
			}
			else
			{
				target = m_render_target;
			}

			switch (m_clear_flags)
			{
			case CameraClearFlags::Invalidate:
				params.flags.clear = filament::backend::TargetBufferFlags::NONE;
				if (m_render_target_color)
				{
					params.flags.discardStart |= filament::backend::TargetBufferFlags::COLOR;
				}
				if (m_render_target_depth)
				{
					params.flags.discardStart |= filament::backend::TargetBufferFlags::DEPTH;
				}
				break;
			case CameraClearFlags::Color:
				if (m_render_target_color)
				{
					params.flags.clear = filament::backend::TargetBufferFlags::COLOR;
				}
				break;
			case CameraClearFlags::Depth:
				if (m_render_target_depth)
				{
					params.flags.clear = filament::backend::TargetBufferFlags::DEPTH;
				}
				break;
			case CameraClearFlags::ColorAndDepth:
				params.flags.clear = filament::backend::TargetBufferFlags::NONE;
				if (m_render_target_color)
				{
					params.flags.clear |= filament::backend::TargetBufferFlags::COLOR;
				}
				if (m_render_target_depth)
				{
					params.flags.clear |= filament::backend::TargetBufferFlags::DEPTH;
				}
				break;
			case CameraClearFlags::Nothing:
				params.flags.clear = filament::backend::TargetBufferFlags::NONE;
				break;
			}
		}
		else
		{
			if (has_post_processing)
			{
				m_post_processing_target = RenderTarget::GetTemporaryRenderTarget(
					target_width,
					target_height,
					TextureFormat::R8G8B8A8,
					Texture::SelectDepthFormat(),
					FilterMode::Linear,
					SamplerAddressMode::ClampToEdge,
					filament::backend::TargetBufferFlags::COLOR_AND_DEPTH);
				target = m_post_processing_target->target;
			}
			else
			{
				target = *(filament::backend::RenderTargetHandle*) Engine::Instance()->GetDefaultRenderTarget();
			}

			switch (m_clear_flags)
			{
			case CameraClearFlags::Invalidate:
				params.flags.clear = filament::backend::TargetBufferFlags::NONE;
				params.flags.discardStart = filament::backend::TargetBufferFlags::COLOR_AND_DEPTH;
				break;
			case CameraClearFlags::Color:
				params.flags.clear = filament::backend::TargetBufferFlags::COLOR;
				break;
			case CameraClearFlags::Depth:
				params.flags.clear = filament::backend::TargetBufferFlags::DEPTH;
				break;
			case CameraClearFlags::ColorAndDepth:
				params.flags.clear = filament::backend::TargetBufferFlags::COLOR_AND_DEPTH;
				break;
			case CameraClearFlags::Nothing:
				params.flags.clear = filament::backend::TargetBufferFlags::NONE;
				break;
			}
		}

		params.viewport.left = (int32_t) (m_viewport_rect.x * target_width);
		params.viewport.bottom = (int32_t) ((1.0f - (m_viewport_rect.y + m_viewport_rect.h)) * target_height);
		params.viewport.width = (uint32_t) (m_viewport_rect.w * target_width);
		params.viewport.height = (uint32_t) (m_viewport_rect.h * target_height);
		params.clearColor = filament::math::float4(m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a);

		driver.beginRenderPass(target, params);

		driver.bindUniformBuffer((size_t) Shader::BindingPoint::PerView, m_view_uniform_buffer);

        for (auto i : renderers)
        {
            this->DrawRenderer(i);
        }
        
		driver.endRenderPass();

		driver.flush();
	}

    void Camera::DrawRenderer(Renderer* renderer)
    {
        auto& driver = Engine::Instance()->GetDriverApi();
		
		driver.bindUniformBuffer((size_t) Shader::BindingPoint::PerRenderer, renderer->GetTransformUniformBuffer());

        SkinnedMeshRenderer* skin = dynamic_cast<SkinnedMeshRenderer*>(renderer);
        if (skin && skin->GetBonesUniformBuffer())
        {
            driver.bindUniformBuffer((size_t) Shader::BindingPoint::PerRendererBones, skin->GetBonesUniformBuffer());
        }

		auto draw = [&](bool light_add = false) {
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
					else if (primitives.Size() > 0)
					{
						primitive = primitives[0];
					}

					if (primitive)
					{
						if (renderer->IsRecieveShadow())
						{
							material->EnableKeyword("RECIEVE_SHADOW_ON");
						}

						const auto& shader = light_add ? material->GetLightAddShader() : material->GetShader();
						
						material->SetScissor(this->GetTargetWidth(), this->GetTargetHeight());

						for (int j = 0; j < shader->GetPassCount(); ++j)
						{
							bool has_light = shader->GetPass(j).light_mode == Shader::LightMode::Forward;
							if (!has_light && light_add)
							{
								continue;
							}

							material->Bind(j);

							const auto& pipeline = shader->GetPass(j).pipeline;
							driver.draw(pipeline, primitive);
						}
					}
				}
			}
		};

		bool lighted = false;
		bool light_add = false;
		const auto& lights = Light::GetLights();
		for (auto i : lights)
		{
			if ((1 << renderer->GetGameObject()->GetLayer()) & i->GetCullingMask())
			{
				if (i->IsShadowEnable())
				{
					if (i->GetViewUniformBuffer())
					{
						driver.bindUniformBuffer((size_t) Shader::BindingPoint::PerLightVertex, i->GetViewUniformBuffer());
					}
					
					if (i->GetSamplerGroup())
					{
						driver.bindSamplers((size_t) Shader::BindingPoint::PerLightFragment, i->GetSamplerGroup());
					}
				}
				driver.bindUniformBuffer((size_t) Shader::BindingPoint::PerLightFragment, i->GetLightUniformBuffer());

				draw(light_add);

				lighted = true;
				light_add = true;
			}
		}

		if (!lighted)
		{
			draw();
		}
    }

	bool Camera::HasPostProcessing()
	{
		Vector<Ref<Viry3D::PostProcessing>> coms = this->GetGameObject()->GetComponents<Viry3D::PostProcessing>();
		return coms.Size() > 0;
	}

	void Camera::PostProcessing()
	{
		Vector<Ref<Viry3D::PostProcessing>> coms = this->GetGameObject()->GetComponents<Viry3D::PostProcessing>();
		if (coms.Size() == 0)
		{
			return;
		}

		int target_width = this->GetTargetWidth();
		int target_height = this->GetTargetHeight();

		Ref<RenderTarget> post_processing_target_1;
		Ref<RenderTarget> post_processing_target_2;
		if (coms.Size() > 1)
		{
			post_processing_target_1 = RenderTarget::GetTemporaryRenderTarget(
				target_width,
				target_height,
				TextureFormat::R8G8B8A8,
				TextureFormat::None,
				FilterMode::Linear,
				SamplerAddressMode::ClampToEdge,
				filament::backend::TargetBufferFlags::COLOR);
		}
		if (coms.Size() > 2)
		{
			post_processing_target_2 = RenderTarget::GetTemporaryRenderTarget(
				target_width,
				target_height,
				TextureFormat::R8G8B8A8,
				TextureFormat::None,
				FilterMode::Linear,
				SamplerAddressMode::ClampToEdge,
				filament::backend::TargetBufferFlags::COLOR);
		}

		Ref<RenderTarget> src = m_post_processing_target;
		Ref<RenderTarget> dst = post_processing_target_1;

		for (int i = 0; i < coms.Size(); ++i)
		{
			if (i == coms.Size() - 1)
			{
				dst = RefMake<RenderTarget>();
				dst->key.width = target_width;
				dst->key.height = target_height;
				dst->key.filter_mode = FilterMode::Nearest;
				dst->key.wrap_mode = SamplerAddressMode::ClampToEdge;

				if (m_render_target_color || m_render_target_depth)
				{
					filament::backend::TargetBufferFlags target_flags = filament::backend::TargetBufferFlags::NONE;
					TextureFormat color_format = TextureFormat::None;
					TextureFormat depth_format = TextureFormat::None;

					if (m_render_target_color)
					{
						target_flags |= filament::backend::TargetBufferFlags::COLOR;
						color_format = m_render_target_color->GetFormat();
					}
					if (m_render_target_depth)
					{
						target_flags |= filament::backend::TargetBufferFlags::DEPTH;
						depth_format = m_render_target_depth->GetFormat();
					}

					dst->key.color_format = color_format;
					dst->key.depth_format = depth_format;
					dst->key.flags = target_flags;

					dst->target = m_render_target;
				}
				else
				{
					dst->key.color_format = TextureFormat::R8G8B8A8;
					dst->key.depth_format = Texture::SelectDepthFormat();
					dst->key.flags = filament::backend::TargetBufferFlags::COLOR_AND_DEPTH;

					dst->target = *(filament::backend::RenderTargetHandle*) Engine::Instance()->GetDefaultRenderTarget();
				}
			}

			coms[i]->SetCameraDepthTexture(m_post_processing_target->depth);
			coms[i]->OnRenderImage(src, dst);
			coms[i]->SetCameraDepthTexture(Ref<Texture>());

			// swap
			if (i < coms.Size() - 1)
			{
				Ref<RenderTarget> temp = src;
				src = dst;
				dst = temp;

				if (i == 0 && coms.Size() > 2)
				{
					dst = post_processing_target_2;
				}
			}
		}

		if (post_processing_target_1)
		{
			RenderTarget::ReleaseTemporaryRenderTarget(post_processing_target_1);
			post_processing_target_1.reset();
		}
		if (post_processing_target_2)
		{
			RenderTarget::ReleaseTemporaryRenderTarget(post_processing_target_2);
			post_processing_target_2.reset();
		}

		RenderTarget::ReleaseTemporaryRenderTarget(m_post_processing_target);
		m_post_processing_target.reset();
	}

	void Camera::Blit(const Ref<RenderTarget>& src, const Ref<RenderTarget>& dst, const Ref<Material>& mat, int pass)
	{
		int target_width = dst->key.width;
		int target_height = dst->key.height;

		filament::backend::RenderPassParams params;
		params.flags.clear = filament::backend::TargetBufferFlags::COLOR;
		if (dst->target == m_current_camera->m_render_target ||
			dst->target == *(filament::backend::RenderTargetHandle*) Engine::Instance()->GetDefaultRenderTarget())
		{
			params.flags.clear = dst->key.flags;
		}

		params.viewport.left = 0;
		params.viewport.bottom = 0;
		params.viewport.width = (uint32_t) target_width;
		params.viewport.height = (uint32_t) target_height;
		params.clearColor = filament::math::float4(0, 0, 0, 0);

		// draw quad
		filament::backend::RenderPrimitiveHandle primitive;
		if (!m_quad_mesh)
		{
			Vector<Mesh::Vertex> vertices(4);
			vertices[0].vertex = Vector3(-1, 1, 0);
			vertices[1].vertex = Vector3(-1, -1, 0);
			vertices[2].vertex = Vector3(1, -1, 0);
			vertices[3].vertex = Vector3(1, 1, 0);
			
			if (Engine::Instance()->GetBackend() == filament::backend::Backend::OPENGL)
			{
				vertices[0].uv = Vector2(0, 1);
				vertices[1].uv = Vector2(0, 0);
				vertices[2].uv = Vector2(1, 0);
				vertices[3].uv = Vector2(1, 1);
			}
			else
			{
				vertices[0].uv = Vector2(0, 0);
				vertices[1].uv = Vector2(0, 1);
				vertices[2].uv = Vector2(1, 1);
				vertices[3].uv = Vector2(1, 0);
			}

			Vector<unsigned int> indices = {
				0, 1, 2, 0, 2, 3
			};

			m_quad_mesh = RefMake<Mesh>(std::move(vertices), std::move(indices));
		}
		primitive = m_quad_mesh->GetPrimitives()[0];

		Ref<Material> material = mat;
		if (!material)
		{
			if (!m_blit_material)
			{
				m_blit_material = RefMake<Material>(Shader::Find("Blit"));
			}
			material = m_blit_material;

			material->SetTexture(MaterialProperty::TEXTURE, src->color);
		}

		if (primitive && material)
		{
			material->Prepare(pass);

			auto& driver = Engine::Instance()->GetDriverApi();
			driver.beginRenderPass(dst->target, params);

			const auto& shader = material->GetShader();
			material->SetScissor(target_width, target_height);

			int pass_begin = 0;
			int pass_end = shader->GetPassCount();
			if (pass >= 0 && pass < shader->GetPassCount())
			{
				pass_begin = pass;
				pass_end = pass + 1;
			}

			for (int i = pass_begin; i < pass_end; ++i)
			{
				material->Bind(i);

				const auto& pipeline = shader->GetPass(i).pipeline;
				driver.draw(pipeline, primitive);
			}

			driver.endRenderPass();
			driver.flush();
		}
	}

	Camera::Camera():
		m_depth(0),
        m_culling_mask(0xffffffff),
		m_clear_flags(CameraClearFlags::ColorAndDepth),
		m_clear_color(0, 0, 0, 1),
		m_viewport_rect(0, 0, 1, 1),
		m_field_of_view(45),
        m_aspect(-1),
		m_near_clip(0.3f),
		m_far_clip(1000),
		m_orthographic(false),
		m_orthographic_size(1),
		m_view_matrix_dirty(true),
		m_projection_matrix_dirty(true),
		m_view_matrix_external(false),
		m_projection_matrix_external(false)
    {
		m_cameras.AddLast(this);
		m_cameras_order_dirty = true;
    }
    
	Camera::~Camera()
    {
		auto& driver = Engine::Instance()->GetDriverApi();

		if (m_view_uniform_buffer)
		{
			driver.destroyUniformBuffer(m_view_uniform_buffer);
			m_view_uniform_buffer.clear();
		}

		if (m_render_target)
		{
			driver.destroyRenderTarget(m_render_target);
			m_render_target.clear();
		}

		m_cameras.Remove(this);
    }

	void Camera::OnTransformDirty()
	{
		m_view_matrix_dirty = true;
	}

	void Camera::SetDepth(int depth)
	{
		m_depth = depth;
	}

    void Camera::SetCullingMask(uint32_t mask)
    {
        m_culling_mask = mask;
    }
    
	void Camera::SetClearFlags(CameraClearFlags flags)
	{
		m_clear_flags = flags;
	}

	void Camera::SetClearColor(const Color& color)
	{
		m_clear_color = color;
	}

	void Camera::SetViewportRect(const Rect& rect)
	{
		m_viewport_rect = rect;
		m_projection_matrix_dirty = true;
	}

	void Camera::SetFieldOfView(float fov)
	{
		m_field_of_view = fov;
		m_projection_matrix_dirty = true;
	}
    
    float Camera::GetAspect() const
    {
        if (m_aspect > 0)
        {
            return m_aspect;
        }
        else
        {
            return this->GetTargetWidth() / (float) this->GetTargetHeight();
        }
    }
    
    void Camera::SetAspect(float aspect)
    {
        m_aspect = aspect;
        m_projection_matrix_dirty = true;
    }

	void Camera::SetNearClip(float clip)
	{
		m_near_clip = clip;
		m_projection_matrix_dirty = true;
	}

	void Camera::SetFarClip(float clip)
	{
		m_far_clip = clip;
		m_projection_matrix_dirty = true;
	}

	void Camera::SetOrthographic(bool enable)
	{
		m_orthographic = enable;
		m_projection_matrix_dirty = true;
	}

	void Camera::SetOrthographicSize(float size)
	{
		m_orthographic_size = size;
		m_projection_matrix_dirty = true;
	}

	const Matrix4x4& Camera::GetViewMatrix()
	{
		if (m_view_matrix_dirty)
		{
			m_view_matrix_dirty = false;

			if (!m_view_matrix_external)
			{
				m_view_matrix = Matrix4x4::LookTo(this->GetTransform()->GetPosition(), this->GetTransform()->GetForward(), this->GetTransform()->GetUp());
			}
		}

		return m_view_matrix;
	}

	const Matrix4x4& Camera::GetProjectionMatrix()
	{
		if (m_projection_matrix_dirty)
		{
			m_projection_matrix_dirty = false;

			if (!m_projection_matrix_external)
			{
				float view_width = this->GetTargetWidth() * m_viewport_rect.w;
				float view_height = this->GetTargetHeight() * m_viewport_rect.h;
                float aspect = m_aspect;
                if (aspect <= 0)
                {
                    aspect = view_width / view_height;
                }

				if (m_orthographic)
				{
					float ortho_size = m_orthographic_size;
					float top = ortho_size;
					float bottom = -ortho_size;
					float plane_h = ortho_size * 2;
					float plane_w = plane_h * aspect;
					m_projection_matrix = Matrix4x4::Ortho(-plane_w / 2, plane_w / 2, bottom, top, m_near_clip, m_far_clip);
				}
				else
				{
					m_projection_matrix = Matrix4x4::Perspective(m_field_of_view, aspect, m_near_clip, m_far_clip);
				}
			}
		}

		return m_projection_matrix;
	}

	void Camera::SetViewMatrixExternal(const Matrix4x4& mat)
	{
		m_view_matrix = mat;
		m_view_matrix_dirty = true;
		m_view_matrix_external = true;
	}

	void Camera::SetProjectionMatrixExternal(const Matrix4x4& mat)
	{
		m_projection_matrix = mat;
		m_projection_matrix_dirty = true;
		m_projection_matrix_external = true;
	}

	void Camera::SetRenderTarget(const Ref<Texture>& color, const Ref<Texture>& depth)
	{
		m_render_target_color = color;
		m_render_target_depth = depth;
		m_projection_matrix_dirty = true;

		auto& driver = Engine::Instance()->GetDriverApi();
		if (m_render_target)
		{
			driver.destroyRenderTarget(m_render_target);
			m_render_target.clear();
		}
	}

	int Camera::GetTargetWidth() const
	{
		if (m_render_target_color)
		{
			return m_render_target_color->GetWidth();
		}
		else if (m_render_target_depth)
		{
			return m_render_target_depth->GetWidth();
		}
		else
		{
			return Engine::Instance()->GetWidth();
		}
	}

	int Camera::GetTargetHeight() const
	{
		if (m_render_target_color)
		{
			return m_render_target_color->GetHeight();
		}
		else if (m_render_target_depth)
		{
			return m_render_target_depth->GetHeight();
		}
		else
		{
			return Engine::Instance()->GetHeight();
		}
	}
}
