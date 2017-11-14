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
#include "renderer/MeshRenderer.h"
#include "graphics/Graphics.h"
#include "graphics/Camera.h"
#include "graphics/Material.h"
#include "physics/BoxCollider.h"
#include "physics/Physics.h"

using namespace Viry3D;

class AppPhysics: public Application
{
public:
	AppPhysics()
	{
		this->SetName("Viry3D::AppPhysics");
		this->SetInitSize(1280, 720);
	}

	virtual void Start()
	{
		this->CreateFPSUI(20, 1, 1);

		auto camera = GameObject::Create("camera")->AddComponent<Camera>();
		camera->SetCullingMask(1 << 0);
		camera->GetTransform()->SetPosition(Vector3(0, 6, -10.0f));
		camera->GetTransform()->SetRotation(Quaternion::Euler(30, 0, 0));
		m_camera = camera;

		auto plane_mesh = Resource::LoadMesh("Assets/Library/unity default resources.Plane.mesh");
		auto sphere_mesh = Resource::LoadMesh("Assets/Library/unity default resources.Sphere.mesh");
		m_cube_mesh = Resource::LoadMesh("Assets/Library/unity default resources.Cube.mesh");

		auto plane = GameObject::Create("plane")->AddComponent<MeshRenderer>();
		plane->SetSharedMaterial(Material::Create("Diffuse"));
		plane->SetSharedMesh(plane_mesh);

		auto plane_col = plane->GetGameObject()->AddComponent<BoxCollider>();
		plane_col->SetSize(Vector3(10, 0, 10));

		auto sphere_mat = Material::Create("Gizmos");
		sphere_mat->SetMainColor(Color(0, 1, 0, 1));

		m_cube_mat = Material::Create("Diffuse");
		m_cube_mat->SetMainColor(Color(1, 0, 0, 1));

		// make weak ref avoid cycle ref
		WeakRef<Camera> camera_weak = camera;
		camera->SetPostRenderFunc([=]() {
			if (m_mouse_down)
			{
				auto pos = Input::GetMousePosition();
				auto ray = camera_weak.lock()->ScreenPointToRay(pos);

				RaycastHit hit;
				if (Physics::Raycast(hit, ray.GetOrigin(), ray.GetDirection(), 1000))
				{
					auto point = hit.point;

					auto sphere_matrix = Matrix4x4::TRS(point, Quaternion::Identity(), Vector3::One() * 0.1f);
					Graphics::DrawMesh(sphere_mesh, sphere_matrix, sphere_mat);
				}
			}
		});
	}

	virtual void Update()
	{
		if (Input::GetMouseButtonDown(0))
		{
			m_mouse_down = true;

			auto pos = Input::GetMousePosition();
			auto ray = m_camera->ScreenPointToRay(pos);

			RaycastHit hit;
			if (Physics::Raycast(hit, ray.GetOrigin(), ray.GetDirection(), 1000))
			{
				auto point = hit.point;

				auto cube = GameObject::Create("cube")->AddComponent<MeshRenderer>();
				cube->GetTransform()->SetPosition(point + Vector3(0, 2, 0));
				cube->GetTransform()->SetScale(Vector3::One() * 0.5f);
				cube->SetSharedMaterial(m_cube_mat);
				cube->SetSharedMesh(m_cube_mesh);

				auto cube_col = cube->GetGameObject()->AddComponent<BoxCollider>();
				cube_col->SetIsRigidbody(true);
			}
		}
		else if (Input::GetMouseButtonUp(0))
		{
			m_mouse_down = false;
		}
	}

	bool m_mouse_down = false;
	Ref<Camera> m_camera;
	Ref<Mesh> m_cube_mesh;
	Ref<Material> m_cube_mat;
};

#if 0
VR_MAIN(AppPhysics);
#endif
