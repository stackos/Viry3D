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
#include "noise/noise.h"
#include "noise/noiseutils.h"

using namespace Viry3D;

class AppTerrain: public Application
{
public:
	AppTerrain();
	virtual void Start();
};

#if 0
VR_MAIN(AppTerrain);
#endif

AppTerrain::AppTerrain()
{
	this->SetName("Viry3D::AppClear");
	this->SetInitSize(1280, 720);
}

void AppTerrain::Start()
{
	GameObject::Create("camera")->AddComponent<Camera>();

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
	heightMapBuilder.SetDestSize(2048, 2048);
	heightMapBuilder.SetBounds(0, 4, 0, 4);
	heightMapBuilder.Build();

	utils::RendererImage renderer;
	utils::Image image;
	renderer.SetSourceNoiseMap(heightMap);
	renderer.SetDestImage(image);
	renderer.ClearGradient();
	renderer.AddGradientPoint(-1.00, utils::Color(32, 160, 0, 255)); // grass
	renderer.AddGradientPoint(-0.25, utils::Color(224, 224, 0, 255)); // dirt
	renderer.AddGradientPoint(0.25, utils::Color(128, 128, 128, 255)); // rock
	renderer.AddGradientPoint(1.00, utils::Color(255, 255, 255, 255)); // snow
	renderer.EnableLight();
	renderer.SetLightContrast(3.0);
	renderer.SetLightBrightness(2.0);
	renderer.Render();

	utils::WriterBMP writer;
	writer.SetSourceImage(image);
	writer.SetDestFilename((Application::SavePath() + "/tutorial.bmp").CString());
	writer.WriteDestFile();
}
