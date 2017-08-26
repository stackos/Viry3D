/*
* Viry3D
* Copyright 2014-2017 by Stack - stackos@qq.com
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

#include "RenderTextureBliter.h"
#include "Graphics.h"
#include "GameObject.h"
#include "Camera.h"
#include "RenderTexture.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(RenderTextureBliter);

	void RenderTextureBliter::DeepCopy(const Ref<Object>& source)
	{
	}

	void RenderTextureBliter::Start()
	{
		GetGameObject()->GetComponent<Camera>()->SetAlwaysRebuildCmd();
	}

	void RenderTextureBliter::OnPostRender()
	{
#if VR_GLES
		bool reverse_uv_y = true;
#else
		bool reverse_uv_y = false;
#endif

		Rect rect(0, 0, 1, 1);
		Graphics::DrawQuad(&rect, rt->color_texture, reverse_uv_y);
	}
}
