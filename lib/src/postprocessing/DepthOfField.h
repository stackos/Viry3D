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

namespace Viry3D
{
	class Material;

	class DepthOfField : public PostProcessing
	{
	public:
		enum class KernelSize
		{
			Small,
			Medium,
			Large,
			VeryLarge,
		};

		DepthOfField();
		virtual ~DepthOfField();
		virtual void OnRenderImage(const Ref<RenderTarget>& src, const Ref<RenderTarget>& dst);
		void SetFocusDistance(float distance) { m_focus_distance = distance; }
		void SetAperture(float aperture) { m_aperture = aperture; }
		void SetFocalLength(float length) { m_focal_length = length; }
		void SetKernelSize(KernelSize size) { m_kernel_size = size; }

	private:
		Ref<Material> m_material;
		float m_focus_distance = 10.0f;
		float m_aperture = 5.6f;
		float m_focal_length = 50.0f;
		KernelSize m_kernel_size = KernelSize::Medium;
	};
}
