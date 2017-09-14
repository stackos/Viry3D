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
#include "noise/noise.h"
#include "noise/noiseutils.h"

using namespace Viry3D;

class AppTerrain: public Application
{
public:
	AppTerrain();
	virtual ~AppTerrain();
	virtual void Start();

	Ref<Texture2D> m_image;
};

#if 1
VR_MAIN(AppTerrain);
#endif

AppTerrain::AppTerrain()
{
	this->SetName("Viry3D::AppTerrain");
	this->SetInitSize(1280, 720);
}

AppTerrain::~AppTerrain()
{
	m_image.reset();
}

void AppTerrain::Start()
{
	auto camera = GameObject::Create("camera")->AddComponent<Camera>();

	module::RidgedMulti mountainTerrain;

	module::Billow baseFlatTerrain;
	baseFlatTerrain.SetFrequency(2.0);

	module::ScaleBias flatTerrain;
	flatTerrain.SetSourceModule(0, baseFlatTerrain);
	flatTerrain.SetScale(0.125);
	flatTerrain.SetBias(-0.75);

	module::Perlin terrainType;
	terrainType.SetFrequency(0.5);
	terrainType.SetPersistence(0.25);

	module::Select terrainSelector;
	terrainSelector.SetSourceModule(0, flatTerrain);
	terrainSelector.SetSourceModule(1, mountainTerrain);
	terrainSelector.SetControlModule(terrainType);
	terrainSelector.SetBounds(0.0, 1000.0);
	terrainSelector.SetEdgeFalloff(0.125);

	module::Turbulence finalTerrain;
	finalTerrain.SetSourceModule(0, terrainSelector);
	finalTerrain.SetFrequency(4.0);
	finalTerrain.SetPower(0.125);

	utils::NoiseMap heightMap;
	utils::NoiseMapBuilderPlane heightMapBuilder;
	heightMapBuilder.SetSourceModule(finalTerrain);
	heightMapBuilder.SetDestNoiseMap(heightMap);
	heightMapBuilder.SetDestSize(513, 513);
	heightMapBuilder.SetBounds(0, 4, 0, 4);
	heightMapBuilder.Build();

	auto stride = heightMap.GetStride();
	auto height_buffer = ByteBuffer(513 * 513);
	for (int i = 0; i < 513; i++)
	{
		auto row = heightMap.GetSlabPtr(i);
		for (int j = 0; j < 513; j++)
		{
			byte a = (byte) Mathf::Min((int) ((row[j] + 1) * 0.5f * 255), 255);
			height_buffer[i * 513 + j] = a;
		}
	}
	m_image = Texture2D::Create(513, 513, TextureFormat::Alpha8, TextureWrapMode::Clamp, FilterMode::Point, false, height_buffer);

	camera->SetPostRenderFunc([=]() {
#if VR_GLES
		bool reverse = true;
#else
		bool reverse = false;
#endif
		Viry3D::Rect rect(0, 0, 1.0f * 9 / 16, 1);
		Graphics::DrawQuad(&rect, m_image, reverse);
	});
}
