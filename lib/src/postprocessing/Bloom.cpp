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

#include "Bloom.h"
#include "graphics/Camera.h"
#include "graphics/Material.h"
#include "graphics/RenderTarget.h"

namespace Viry3D
{
	Bloom::Bloom()
	{
		m_material = RefMake<Material>(Shader::Find("PostProcessing/Bloom"));
	}

	Bloom::~Bloom()
	{
		m_material.reset();
	}

	void Bloom::OnRenderImage(const Ref<RenderTarget>& src, const Ref<RenderTarget>& dst)
	{
		const int MAX_PYRAMID_SIZE = 16;

		enum Pass
		{
			Prefilter13,
			Downsample13,
			UpsampleTent,
            Uber,
		};

		struct Level
		{
			Ref<RenderTarget> down;
			Ref<RenderTarget> up;
		};

		float ratio = Mathf::Clamp(m_anamorphic_ratio, -1.0f, 1.0f);
		float rw = ratio < 0 ? -ratio : 0;
		float rh = ratio > 0 ?  ratio : 0;

		// half res
		int tw = Mathf::FloorToInt(dst->key.width / (2 - rw));
		int th = Mathf::FloorToInt(dst->key.height / (2 - rh));

		// determine the iteration count
		int s = Mathf::Max(tw, th);
		float logs = Mathf::Log2((float) s) + Mathf::Min(m_diffusion, 10.0f) - 10;
		int logs_i = Mathf::FloorToInt(logs); 
		int iterations = Mathf::Clamp(logs_i, 1, MAX_PYRAMID_SIZE);
		float sample_scale = 0.5f + logs - logs_i;
        m_material->SetFloat("_SampleScale", sample_scale);
        
		// prefiltering parameters
		float lthresh = m_threshold;
		float knee = lthresh * m_soft_knee + 1e-5f;
		m_material->SetVector("_Threshold", Vector4(lthresh, lthresh - knee, knee * 2, 0.25f / knee));
		float lclamp = m_clamp;
		m_material->SetVector("_Params", Vector4(lclamp, 0, 0, 0));

		// downsample
		Level levels[MAX_PYRAMID_SIZE];
		auto last_down = src;
		for (int i = 0; i < iterations; i++)
		{
			int pass = i == 0 ? (int) Pass::Prefilter13 : (int) Pass::Downsample13;

			levels[i].down = RenderTarget::GetTemporaryRenderTarget(
				tw,
				th,
				src->key.color_format,
				TextureFormat::None,
				FilterMode::Linear,
				SamplerAddressMode::ClampToEdge,
				filament::backend::TargetBufferFlags::COLOR);
			levels[i].up = RenderTarget::GetTemporaryRenderTarget(
				tw,
				th,
				src->key.color_format,
				TextureFormat::None,
				FilterMode::Linear,
				SamplerAddressMode::ClampToEdge,
				filament::backend::TargetBufferFlags::COLOR);
			m_material->SetTexture(MaterialProperty::TEXTURE, last_down->color);
			m_material->SetVector("u_texel_size", Vector4(1.0f / last_down->color->GetWidth(), 1.0f / last_down->color->GetHeight(), 0, 0));
			Camera::Blit(last_down, levels[i].down, m_material, pass);

			last_down = levels[i].down;
			tw = Mathf::Max(tw / 2, 1);
			th = Mathf::Max(th / 2, 1);
		}

		// upsample
		auto last_up = levels[iterations - 1].down;
		for (int i = iterations - 2; i >= 0; i--)
		{
			m_material->SetTexture(MaterialProperty::TEXTURE, last_up->color);
            m_material->SetVector("u_texel_size", Vector4(1.0f / last_up->color->GetWidth(), 1.0f / last_up->color->GetHeight(), 0, 0));
            m_material->SetTexture("_BloomTex", levels[i].down->color);
			Camera::Blit(last_up, levels[i].up, m_material, (int) Pass::UpsampleTent);

			last_up = levels[i].up;
		}

        // uber
        m_material->SetTexture(MaterialProperty::TEXTURE, src->color);
        m_material->SetVector("_Bloom_Settings", Vector4(sample_scale, m_intensity, 0, (float) iterations));
        m_material->SetColor("_Bloom_Color", m_color);
        m_material->SetTexture("_BloomTex", last_up->color);
        m_material->SetVector("u_texel_size", Vector4(1.0f / last_up->color->GetWidth(), 1.0f / last_up->color->GetHeight(), 0, 0));
        Camera::Blit(src, dst, m_material, (int) Pass::Uber);
        
		// cleanup
		for (int i = 0; i < iterations; i++)
		{
			RenderTarget::ReleaseTemporaryRenderTarget(levels[i].down);
			RenderTarget::ReleaseTemporaryRenderTarget(levels[i].up);
		}
	}
}
