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

#include "Main.h"
#include "Application.h"
#include "GameObject.h"
#include "Resource.h"
#include "Input.h"
#include "Layer.h"
#include "graphics/Graphics.h"
#include "graphics/Camera.h"
#include "graphics/Material.h"
#include "graphics/Cubemap.h"
#include "graphics/Texture2D.h"
#include "graphics/Light.h"
#include "renderer/MeshRenderer.h"
#include "io/File.h"
#include "ui/UIEventHandler.h"
#include "DebugUI.h"

using namespace Viry3D;

class AppPBR: public Application
{
public:
	AppPBR()
	{
		this->SetName("Viry3D::AppPBR");
		this->SetInitSize(1280, 720);
	}

	~AppPBR()
	{
		Graphics::GetDisplay()->EndRecord();
	}

	virtual void Start()
	{
		//Graphics::GetDisplay()->BeginRecord("../../../demo.mp4");

		// debug ui
		{
			auto ui_camera = GameObject::Create("camera")->AddComponent<Camera>();
			ui_camera->SetCullingMask(1 << (int) Layer::UI);
			ui_camera->SetOrthographic(true);
			ui_camera->SetOrthographicSize(ui_camera->GetTargetHeight() / 2.0f);
			ui_camera->SetClipNear(-1);
			ui_camera->SetClipFar(1);
			ui_camera->SetClearFlags(CameraClearFlags::Nothing);
			ui_camera->SetDepth(1);

			DebugUI::RegisterComponent();
			GameObject::Create("debug_ui")->AddComponent<DebugUI>();
		}

		m_camera = GameObject::Create("camera")->AddComponent<Camera>();
		m_camera->SetCullingMask(1 << 0);
		m_camera->GetTransform()->SetRotation(Quaternion::Euler(m_cam_rot));
		auto cam_dir = m_camera->GetTransform()->GetForward();
		m_camera->GetTransform()->SetPosition(Vector3::Zero() - cam_dir * m_cam_dis);

		auto cubemap = Resource::LoadTexture("Assets/AppPBR/prefilter.asset.tex");

		// skybox
		{
			auto cube_mesh = Resource::LoadMesh("Assets/Library/unity default resources.Cube.mesh");

			m_sky_mat = Material::Create("SkyboxGamma");
			m_sky_mat->SetTexture("_CubeMap", cubemap);
			auto sky_matrix = Matrix4x4::Translation(m_camera->GetTransform()->GetPosition());
			m_sky_mat->SetMatrix("_SkyWorld", sky_matrix);

			auto sky = GameObject::Create("sky")->AddComponent<MeshRenderer>();
			sky->SetSharedMaterial(m_sky_mat);
			sky->SetSharedMesh(cube_mesh);
		}

		Resource::LoadGameObject("Assets/AppPBR/models.prefab");

		auto light = GameObject::Create("light")->AddComponent<Light>();
		light->GetTransform()->SetRotation(Quaternion::Euler(45, -30, 0));
		Light::main = light;
	}

	virtual void Update()
	{
		if (Input::GetMouseButtonDown(0) && !UIEventHandler::HasEvent())
		{
			m_mouse_down = true;
			m_mouse = Input::GetMousePosition();
		}
		else if(Input::GetMouseButtonUp(0))
		{
			m_mouse_down = false;
		}

		if (m_mouse_down)
		{
			auto mouse = Input::GetMousePosition();

			if (m_mouse.x < 0)
			{
				m_mouse = mouse;
			}

			auto delta = mouse - m_mouse;
			m_mouse = mouse;

			if (delta.SqrMagnitude() > 0)
			{
				m_cam_rot.x += -delta.y;
				m_cam_rot.y += delta.x;

				m_camera->GetTransform()->SetRotation(Quaternion::Euler(m_cam_rot));

				auto cam_dir = m_camera->GetTransform()->GetForward();
				m_camera->GetTransform()->SetPosition(Vector3::Zero() - cam_dir * m_cam_dis);

				auto sky_matrix = Matrix4x4::Translation(m_camera->GetTransform()->GetPosition());
				m_sky_mat->SetMatrix("_SkyWorld", sky_matrix);
			}
		}
	}

	Ref<Camera> m_camera;
	Vector3 m_mouse = Vector3(-1, -1, -1);
	Vector3 m_cam_rot = Vector3(30, 0, 0);
	const float m_cam_dis = 4;
	Ref<Material> m_sky_mat;
	bool m_mouse_down = false;
};

#if 1
VR_MAIN(AppPBR);
#endif
