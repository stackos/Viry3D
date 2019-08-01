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

#include "ShowDepth.h"
#include "GameObject.h"
#include "graphics/Camera.h"
#include "graphics/Material.h"
#include "graphics/RenderTarget.h"

namespace Viry3D
{
	ShowDepth::ShowDepth()
	{
		m_material = RefMake<Material>(Shader::Find("PostProcessing/ShowDepth"));
	}

	ShowDepth::~ShowDepth()
	{
		m_material.reset();
	}

	void ShowDepth::OnRenderImage(const Ref<RenderTarget>& src, const Ref<RenderTarget>& dst)
	{
		m_material->SetTexture(MaterialProperty::TEXTURE, this->GetCameraDepthTexture());

		auto camera = this->GetGameObject()->GetComponent<Camera>();
		float near_clip = camera->GetNearClip();
		float far_clip = camera->GetFarClip();
		float zc0 = (1.0f - far_clip / near_clip) / 2.0f;
		float zc1 = (1.0f + far_clip / near_clip) / 2.0f;
		m_material->SetVector("_ZBufferParams", Vector4(zc0, zc1, zc0 / far_clip, zc1 / far_clip));

		Camera::Blit(src, dst, m_material);
	}
}
