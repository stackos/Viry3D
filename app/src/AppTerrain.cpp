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
#include "graphics/Light.h"
#include "renderer/Terrain.h"

using namespace Viry3D;

class AppTerrain: public Application
{
public:
	AppTerrain()
	{
		this->SetName("Viry3D::AppTerrain");
		this->SetInitSize(1280, 720);
	}

	virtual void Start()
	{
		auto camera = GameObject::Create("camera")->AddComponent<Camera>();
		camera->GetTransform()->SetPosition(Vector3(250, 200, -200));
		camera->GetTransform()->SetRotation(Quaternion::Euler(30, 0, 0));

		auto terrain = Resource::LoadGameObject("Assets/AppTerrain/Terrain.prefab")->GetComponent<Terrain>();

		auto light = GameObject::Create("light")->AddComponent<Light>();
		light->GetTransform()->SetRotation(Quaternion::Euler(45, -45, 0));
		Light::main = light;

		bool gen_noise_terrain = true;
		if (gen_noise_terrain)
		{
			terrain->GenerateTile(0, 0);
			terrain->SetTerrainSize(Vector3(500, 50, 500));
			terrain->SetHeightmapData(terrain->GetTile()->height_map_data);
			terrain->Apply();
		}
	}
};

#if 0
VR_MAIN(AppTerrain);
#endif
