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

#pragma once

#include "PostProcessing.h"
#include "graphics/Color.h"

namespace Viry3D
{
	class Material;

	class Bloom : public PostProcessing
	{
	public:
		Bloom();
		virtual ~Bloom();
		virtual void OnRenderImage(const Ref<RenderTarget>& src, const Ref<RenderTarget>& dst);
		void SetIntensity(float intensity) { m_intensity = intensity; }
		void SetThreshold(float threshold) { m_threshold = threshold; }
		void SetSoftKnee(float soft_knee) { m_soft_knee = soft_knee; }
		void SetClamp(float clamp) { m_clamp = clamp; }
		void SetDiffusion(float diffusion) { m_diffusion = diffusion; }
		void SetAnamorphicRatio(float anamorphic_ratio) { m_anamorphic_ratio = anamorphic_ratio; }
		void SetColor(float color) { m_color = color; }

	private:
		Ref<Material> m_material;
		float m_intensity = 0.0f;
		float m_threshold = 1.0f;
		float m_soft_knee = 0.5f;
		float m_clamp = 65472.0f;
		float m_diffusion = 7.0f;
		float m_anamorphic_ratio = 0;
		Color m_color = Color(1, 1, 1, 1);
	};
}
