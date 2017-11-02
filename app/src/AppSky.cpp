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

		auto camera = GameObject::Create("camera")->AddComponent<Camera>();
		camera->GetTransform()->SetPosition(Vector3(0, 2.0f, -2.0f));
		camera->GetTransform()->SetRotation(Quaternion::Euler(30, 0, 0));

		auto plane_mesh = Resource::LoadMesh("Assets/Library/unity default resources.Plane.mesh");
		plane_mesh->Update();

		auto plane = GameObject::Create("plane")->AddComponent<MeshRenderer>();
		plane->SetSharedMaterial(Material::Create("Diffuse"));
		plane->SetSharedMesh(plane_mesh);

		auto sphere_mesh = Resource::LoadMesh("Assets/Library/unity default resources.Sphere.mesh");
		sphere_mesh->Update();

		auto cubemap = Cubemap::Create(512, TextureFormat::RGB24, TextureWrapMode::Clamp, FilterMode::Bilinear, true);
		for (int i = 0; i < 6; i++)
		{
			int width;
			int height;
			ByteBuffer colors;
			TextureFormat format;

			auto file = String::Format("%s/%d.jpg", (Application::DataPath() + "/AppSky").CString(), i);
			auto buffer = File::ReadAllBytes(file);

			if (Texture2D::LoadImageData(buffer, colors, width, height, format))
			{
				cubemap->SetPixels(colors, (CubemapFace) i, 0);
			}
		}
		cubemap->Apply(true, true);

		auto sphere_mat = Material::Create("Reflect");
		sphere_mat->SetTexture("_ReflectMap", cubemap);
		sphere_mat->SetVector("_WorldCameraPos", camera->GetTransform()->GetPosition());
		
		auto sphere = GameObject::Create("sphere")->AddComponent<MeshRenderer>();
		sphere->GetTransform()->SetPosition(Vector3(0, 1, 0));
		sphere->SetSharedMaterial(sphere_mat);
		sphere->SetSharedMesh(sphere_mesh);
	}
};

#if 0
VR_MAIN(AppSky);
#endif
