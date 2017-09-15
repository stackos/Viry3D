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
#include "graphics/Camera.h"
#include "graphics/Texture2D.h"
#include "graphics/Graphics.h"
#include "renderer/Terrain.h"
#include "noise/noise.h"
#include "noise/noiseutils.h"

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
		this->CreateFPSUI(20, 1, 1);

		auto camera = GameObject::Create("camera")->AddComponent<Camera>();
		auto terrain = GameObject::Create("Terrain")->AddComponent<Terrain>();
		auto tile = terrain->GenerateTile(0, 0);

		camera->SetPostRenderFunc([=]() {
#if VR_GLES
			bool reverse = true;
#else
			bool reverse = false;
#endif
			Rect rect(0.5f, 0, 0.5f, 1);
			Graphics::DrawQuad(&rect, tile->debug_image, reverse);
		});
	}
};

#if 1
VR_MAIN(AppTerrain);
#endif
