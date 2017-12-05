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
#include "graphics/Graphics.h"
#include "graphics/Material.h"
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
		camera->GetTransform()->SetPosition(Vector3(253, 17, 140));
		camera->GetTransform()->SetRotation(Quaternion::Euler(30, 0, 0));

		/*auto terrain = GameObject::Create("terrain")->AddComponent<Terrain>();
		terrain->SetHeightmapSize(513);
		terrain->GenerateTile(0, 0);

		camera->SetPostRenderFunc([=]() {
#if VR_GLES
			bool reverse = true;
#else
			bool reverse = false;
#endif
			Viry3D::Rect rect(0.5f, 0, 0.5f, 1);
			Graphics::DrawQuad(&rect, terrain->GetTile()->debug_image, reverse);
		});*/

		auto terrain = Resource::LoadGameObject("Assets/AppTerrain/Terrain.prefab")->GetComponent<Terrain>();
		auto terrain_size = terrain->GetTerrainSize();
		auto splats = terrain->GetSplatTextures();
		auto alphamaps = terrain->GetAlphamaps();
		auto terrain_mat = Material::Create("Terrain/Diffuse");
		for (int i = 0; i < splats.Size(); i++)
		{
			terrain_mat->SetTexture(String::Format("_SplatTex%d", i), splats[i].texture);
			terrain_mat->SetTexture(String::Format("_SplatNormal%d", i), splats[i].normal);
			terrain_mat->SetVector(String::Format("_SplatTex%dSizeOffset", i), Vector4(splats[i].tile_size.x, splats[i].tile_size.y, splats[i].tile_offset.x, splats[i].tile_offset.y));
		}
		if (alphamaps.Size() > 0)
		{
			terrain_mat->SetTexture("_ControlTex0", alphamaps[0]);
			terrain_mat->SetVector("_ControlTex0SizeOffset", Vector4(terrain_size.x, terrain_size.z, 0, 0));
		}
		terrain->SetSharedMaterial(terrain_mat);

		auto light = GameObject::Create("light")->AddComponent<Light>();
		light->GetTransform()->SetRotation(Quaternion::Euler(45, -45, 0));
		Light::main = light;
	}
};

#if 0
VR_MAIN(AppTerrain);
#endif
