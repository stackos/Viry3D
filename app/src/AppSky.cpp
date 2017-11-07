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
#include "graphics/Camera.h"
#include "renderer/MeshRenderer.h"
#include "graphics/Material.h"
#include "graphics/Cubemap.h"
#include "graphics/Texture2D.h"
#include "io/File.h"

using namespace Viry3D;

class AppSky: public Application
{
public:
	AppSky()
	{
		this->SetName("Viry3D::AppSky");
		this->SetInitSize(1280, 720);
	}

	virtual void Start()
	{
		this->CreateFPSUI(20, 1, 1);

		m_camera = GameObject::Create("camera")->AddComponent<Camera>();
		m_camera->SetCullingMask(1 << 0);
		m_camera->GetTransform()->SetRotation(Quaternion::Euler(m_cam_rot));
		auto cam_dir = m_camera->GetTransform()->GetForward();
		m_camera->GetTransform()->SetPosition(Vector3::Zero() - cam_dir * m_cam_dis);

		auto sphere_mesh = Resource::LoadMesh("Assets/Library/unity default resources.Sphere.mesh");
		sphere_mesh->Update();

		auto cubemap = Cubemap::Create(1024, TextureFormat::RGB24, TextureWrapMode::Clamp, FilterMode::Bilinear, false);
		for (int i = 0; i < 6; i++)
		{
			int width;
			int height;
			ByteBuffer colors;
			TextureFormat format;

			auto file = String::Format("%s/dawn/%d.png", (Application::DataPath() + "/AppSky").CString(), i);
			auto buffer = File::ReadAllBytes(file);

			if (Texture2D::LoadImageData(buffer, colors, width, height, format))
			{
				cubemap->SetPixels(colors, (CubemapFace) i, 0);
			}
		}
		cubemap->Apply(false, true);

		m_sphere_mat = Material::Create("Reflect");
		m_sphere_mat->SetTexture("_CubeMap", cubemap);
		m_sphere_mat->SetVector("_WorldCameraPos", m_camera->GetTransform()->GetPosition());
		
		auto sphere = GameObject::Create("sphere")->AddComponent<MeshRenderer>();
		sphere->GetTransform()->SetPosition(Vector3(0, 0, 0));
		sphere->SetSharedMaterial(m_sphere_mat);
		sphere->SetSharedMesh(sphere_mesh);

		// skybox
		auto cube_mesh = Resource::LoadMesh("Assets/Library/unity default resources.Cube.mesh");
		cube_mesh->Update();

		m_sky_mat = Material::Create("Skybox");
		m_sky_mat->SetTexture("_CubeMap", cubemap);
		auto sky_matrix = Matrix4x4::Translation(m_camera->GetTransform()->GetPosition());
		m_sky_mat->SetMatrix("_SkyWorld", sky_matrix);

		auto sky = GameObject::Create("sky")->AddComponent<MeshRenderer>();
		sky->SetSharedMaterial(m_sky_mat);
		sky->SetSharedMesh(cube_mesh);
	}

	virtual void Update()
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

			m_sphere_mat->SetVector("_WorldCameraPos", m_camera->GetTransform()->GetPosition());
		}
	}

	Ref<Camera> m_camera;
	Vector3 m_mouse = Vector3(-1, -1, -1);
	Vector3 m_cam_rot = Vector3(30, 0, 0);
	const float m_cam_dis = 3;
	Ref<Material> m_sky_mat;
	Ref<Material> m_sphere_mat;
};

#if 1
VR_MAIN(AppSky);
#endif
