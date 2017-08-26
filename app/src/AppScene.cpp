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
#include "graphics/LightmapSettings.h"
#include "graphics/Light.h"
#include "graphics/RenderTexture.h"
#include "graphics/RenderTextureBliter.h"
#include "graphics/Graphics.h"
#include "renderer/Renderer.h"
#include "tweener/TweenPosition.h"

using namespace Viry3D;

class AppScene: public Application
{
public:
	AppScene();
	virtual ~AppScene();
	virtual void Start();
	virtual void Update();
};

#if 1
VR_MAIN(AppScene);
#endif

AppScene::AppScene()
{
	this->SetName("Viry3D::AppScene");
	this->SetInitSize(1280, 720);
	//this->SetInitFPS(30);
}

AppScene::~AppScene()
{
	Graphics::GetDisplay()->EndRecord();
}

void AppScene::Start()
{
#define USE_RT 0

#if USE_RT
	int w = 1920;
	int h = 1080;
	auto rt = RefMake<FrameBuffer>();
	rt->color_texture = RenderTexture::Create(w, h, RenderTextureFormat::RGBA32, DepthBuffer::Depth_0, FilterMode::Bilinear);
	rt->depth_texture = RenderTexture::Create(w, h, RenderTextureFormat::Depth, DepthBuffer::Depth_24, FilterMode::Bilinear);

	auto camera_blit = GameObject::Create("camera_blit")->AddComponent<Camera>();
	camera_blit->SetClearFlags(CameraClearFlags::Color);
	camera_blit->SetClearColor(Color(1, 0, 0, 1));
	camera_blit->SetCullingMask(1 << 2);
	camera_blit->SetDepth(2);
	auto post = camera_blit->GetGameObject()->AddComponent<RenderTextureBliter>();
	post->rt = rt;

	this->CreateFPSUI(20, 1, 1, rt);
#else
	this->CreateFPSUI(20, 1, 1);
#endif

	auto camera = GameObject::Create("camera")->AddComponent<Camera>();
	//camera->GetTransform()->SetPosition(Vector3(141, 27, 109));
	//camera->GetTransform()->SetRotation(Quaternion::Euler(44, 60, 0));
	camera->GetTransform()->SetPosition(Vector3(90, 28, 134));
	camera->GetTransform()->SetRotation(Quaternion::Euler(30, -80, 0));
	camera->SetCullingMask(1 << 0);
	camera->SetFieldOfView(40);

#if USE_RT
	camera->SetFrameBuffer(rt);
#endif

	//Resource::LoadLightmapSettings("Assets/AppScene/zhu_cheng.lightmap");
	Resource::LoadLightmapSettings("Assets/AppScene/xiao_zhu_lin.lightmap");

	auto light = GameObject::Create("light")->AddComponent<Light>();
	//light->GetTransform()->SetRotation(Quaternion::Euler(45, 237.57f, 0));
	//light->color = Color(165, 165, 165, 255) / 255.0f;
	light->GetTransform()->SetRotation(Quaternion::Euler(45, -30, 0));
	light->color = Color(255, 208, 167, 255) / 255.0f;
	Light::main = light;

	//Resource::LoadGameObject("Assets/AppScene/zhu_cheng.prefab", true);
	Resource::LoadGameObject("Assets/AppScene/xiao_zhu_lin.prefab", true);

	/*Resource::LoadGameObject("Assets/AppScene/Effect.prefab", false,
		[=](const Ref<Object>& obj) {
		auto t = RefCast<GameObject>(obj)->GetTransform();
		t->SetParent(camera->GetTransform());
		t->SetLocalPosition(Vector3::Zero());
		t->SetLocalRotation(Quaternion::Identity());
		t->SetLocalScale(Vector3::One());
	});*/

	//Graphics::GetDisplay()->BeginRecord("../../../demo.mp4");
}

void AppScene::Update()
{
}