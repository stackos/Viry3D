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

#include "DepthOfField.h"
#include "GameObject.h"
#include "graphics/Camera.h"
#include "graphics/Material.h"
#include "graphics/RenderTarget.h"

namespace Viry3D
{
	DepthOfField::DepthOfField()
	{
		m_material = RefMake<Material>(Shader::Find("PostProcessing/DepthOfField"));
	}

	DepthOfField::~DepthOfField()
	{
		m_material.reset();
	}

	void DepthOfField::OnRenderImage(const Ref<RenderTarget>& src, const Ref<RenderTarget>& dst)
	{
		enum Pass
		{
			CoCCalculation,
			DownsampleAndPrefilter,
			BokehSmallKernel,
			BokehMediumKernel,
			//BokehLargeKernel,
			//BokehVeryLargeKernel,
			PostFilter,
			Combine,
		};

		const float FILM_HEIGHT = 0.024f;

		TextureFormat color_format = TextureFormat::R8G8B8A8;
		TextureFormat coc_format = TextureFormat::R8;

		// material setup
		float f = m_focal_length / 1000.0f;
		float s1 = Mathf::Max(m_focus_distance, f);
		float aspect = src->key.width / (float) src->key.height;
		float coeff = f * f / (m_aperture * (s1 - f) * FILM_HEIGHT * 2.0f);
		float radius_in_pixels = (float) m_kernel_size * 4 + 6;
		float max_coc = Mathf::Min(0.05f, radius_in_pixels / src->key.height);
		float rcp_max_coc = 1.0f / max_coc;
		float rcp_aspect = 1.0f / aspect;

		m_material->SetFloat("_Distance", s1);
		m_material->SetFloat("_LensCoeff", coeff);
		m_material->SetFloat("_MaxCoC", max_coc);
		m_material->SetFloat("_RcpMaxCoC", rcp_max_coc);
		m_material->SetFloat("_RcpAspect", rcp_aspect);

		// coc calculation pass
		m_material->SetTexture(MaterialProperty::TEXTURE, this->GetCameraDepthTexture());

		auto camera = this->GetGameObject()->GetComponent<Camera>();
		float near_clip = camera->GetNearClip();
		float far_clip = camera->GetFarClip();
		float zc0 = 1.0f - far_clip / near_clip;
		float zc1 = far_clip / near_clip;
		m_material->SetVector("_ZBufferParams", Vector4(zc0, zc1, zc0 / far_clip, zc1 / far_clip));

		auto coc_tex = RenderTarget::GetTemporaryRenderTarget(
			src->key.width,
			src->key.height,
			coc_format,
			TextureFormat::None,
			FilterMode::Linear,
			SamplerAddressMode::ClampToEdge,
			filament::backend::TargetBufferFlags::COLOR);
		Camera::Blit(Ref<RenderTarget>(), coc_tex, m_material, (int) Pass::CoCCalculation);

		// downsampling and prefiltering pass
		auto dof_tex = RenderTarget::GetTemporaryRenderTarget(
			src->key.width / 2,
			src->key.height / 2,
			color_format,
			TextureFormat::None,
			FilterMode::Linear,
			SamplerAddressMode::ClampToEdge,
			filament::backend::TargetBufferFlags::COLOR);
		m_material->SetTexture("_CoCTex", coc_tex->color);
		m_material->SetVector("u_texel_size", Vector4(1.0f / src->color->GetWidth(), 1.0f / src->color->GetHeight(), 0, 0));
		m_material->SetTexture(MaterialProperty::TEXTURE, src->color);
		Camera::Blit(src, dof_tex, m_material, (int) Pass::DownsampleAndPrefilter);

		// bokeh simulation pass
		auto dof_temp = RenderTarget::GetTemporaryRenderTarget(
			src->key.width / 2,
			src->key.height / 2,
			color_format,
			TextureFormat::None,
			FilterMode::Linear,
			SamplerAddressMode::ClampToEdge,
			filament::backend::TargetBufferFlags::COLOR);
		m_material->SetVector("u_texel_size", Vector4(1.0f / dof_tex->color->GetWidth(), 1.0f / dof_tex->color->GetHeight(), 0, 0));
		m_material->SetTexture(MaterialProperty::TEXTURE, dof_tex->color);
		Camera::Blit(dof_tex, dof_temp, m_material, (int) Pass::BokehSmallKernel + (int) m_kernel_size);

		// postfilter pass
		m_material->SetVector("u_texel_size", Vector4(1.0f / dof_temp->color->GetWidth(), 1.0f / dof_temp->color->GetHeight(), 0, 0));
		m_material->SetTexture(MaterialProperty::TEXTURE, dof_temp->color);
		Camera::Blit(dof_temp, dof_tex, m_material, (int) Pass::PostFilter);

		// combine pass
		m_material->SetTexture("_CoCTex", coc_tex->color);
		m_material->SetTexture("_DofTex", dof_tex->color);
		m_material->SetVector("u_texel_size", Vector4(1.0f / src->color->GetWidth(), 1.0f / src->color->GetHeight(), 0, 0));
		m_material->SetTexture(MaterialProperty::TEXTURE, src->color);
		Camera::Blit(src, dst, m_material, (int) Pass::Combine);

		RenderTarget::ReleaseTemporaryRenderTarget(dof_temp);
		RenderTarget::ReleaseTemporaryRenderTarget(dof_tex);
		RenderTarget::ReleaseTemporaryRenderTarget(coc_tex);
	}
}
