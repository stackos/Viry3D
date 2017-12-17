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

#include "CodeEditor.h"
#include "GameObject.h"
#include "graphics/Camera.h"
#include "graphics/RenderTexture.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(CodeEditor);

	void CodeEditor::DeepCopy(const Ref<Object>& source)
	{
		Component::DeepCopy(source);
	}

	CodeEditor::CodeEditor():
		m_target_screen_width(0),
		m_target_screen_height(0),
		m_render_depth(0)
	{
	}

	void CodeEditor::SetRenderDepth(int depth)
	{
		m_render_depth = depth;
	}

	void CodeEditor::SetTargetScreenSize(int width, int height)
	{
		m_target_screen_width = width;
		m_target_screen_height = height;
	}

	void CodeEditor::CreateCamera()
	{
		if (m_target_screen_width > 0 &&
			m_target_screen_height > 0)
		{
			int layer = this->GetGameObject()->GetLayer();

			auto camera = GameObject::Create("Camera")->AddComponent<Camera>();
			camera->GetTransform()->SetParent(this->GetTransform());
			camera->GetGameObject()->SetLayer(layer);
			camera->SetCullingMask(1 << layer);
			camera->SetDepth(m_render_depth);
			camera->SetClearColor(Color(30, 30, 30, 255) / 255.0f);

			auto render_target = RefMake<FrameBuffer>();
			render_target->color_texture = RenderTexture::Create(m_target_screen_width, m_target_screen_height, RenderTextureFormat::RGBA32, DepthBuffer::Depth_0, FilterMode::Bilinear);
			render_target->depth_texture = RenderTexture::Create(m_target_screen_width, m_target_screen_height, RenderTextureFormat::Depth, DepthBuffer::Depth_24, FilterMode::Bilinear);
			camera->SetFrameBuffer(render_target);

			camera->SetOrthographic(true);
			camera->SetOrthographicSize(camera->GetTargetHeight() / 2.0f);
			camera->SetClipNear(-1);
			camera->SetClipFar(1);

			m_camera = camera;
		}
	}

	Ref<RenderTexture> CodeEditor::GetTargetRenderTexture() const
	{
		Ref<RenderTexture> texture;

		if (m_camera)
		{
			texture = m_camera->GetFrameBuffer()->color_texture;
		}

		return texture;
	}
}
