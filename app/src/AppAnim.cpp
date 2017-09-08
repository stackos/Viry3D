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
#include "Debug.h"
#include "graphics/Camera.h"
#include "graphics/RenderTexture.h"
#include "graphics/RenderTextureBliter.h"
#include "animation/Animation.h"
#include "ui/UILabel.h"
#include "ui/UISprite.h"
#include "ui/UICanvasRenderer.h"
#include "time/Time.h"
#include "renderer/SkinnedMeshRenderer.h"

using namespace Viry3D;

class AppAnim : public Application
{
public:
	AppAnim();
	virtual void Start();
};

#if 0
VR_MAIN(AppAnim);
#endif

AppAnim::AppAnim()
{
	this->SetName("Viry3D::AppAnim");
	this->SetInitSize(1280, 720);
}

void AppAnim::Start()
{
	auto camera = GameObject::Create("camera")->AddComponent<Camera>();
	camera->SetCullingMask(1 << 0);
	camera->GetTransform()->SetPosition(Vector3(0, 1.2f, -2.0f));
	camera->GetTransform()->SetRotation(Quaternion::Euler(10, 0, 0));

#if VR_WINDOWS
	int w = 1920;
	int h = 1080;
#else
	int w = 1080;
	int h = 1920;
#endif
	auto rt = RefMake<FrameBuffer>();
	rt->color_texture = RenderTexture::Create(w, h, RenderTextureFormat::RGBA32, DepthBuffer::Depth_0, FilterMode::Bilinear);
	rt->depth_texture = RenderTexture::Create(w, h, RenderTextureFormat::Depth, DepthBuffer::Depth_24, FilterMode::Bilinear);
	camera->SetFrameBuffer(rt);

	this->CreateFPSUI(20, 1, 1, rt);

	auto obj = Resource::LoadGameObject("Assets/AppAnim/unitychan.prefab");
	obj->GetTransform()->SetRotation(Quaternion::Euler(0, 180, 0));
	auto anim = obj->GetComponent<Animation>();
	auto state = anim->GetAnimationState("WAIT03");
	state.wrap_mode = AnimationWrapMode::Loop;
	anim->UpdateAnimationState("WAIT03", state);
	anim->Play("WAIT03");

#if true // blit
	auto camera2 = GameObject::Create("camera2")->AddComponent<Camera>();
	camera2->SetClearFlags(CameraClearFlags::Color);
	camera2->SetClearColor(Color(1, 0, 0, 1));
	camera2->SetCullingMask(1 << 2);
	camera2->SetDepth(2);

	auto post = camera2->GetGameObject()->AddComponent<RenderTextureBliter>();
	post->rt = rt;
#else // as 2d sprite
	auto camera2 = GameObject::Create("camera2")->AddComponent<Camera>();
	camera2->SetClearFlags(CameraClearFlags::Color);
	camera2->SetClearColor(Color(1, 0, 0, 1));
	camera2->SetCullingMask(1 << 2);
	camera2->SetDepth(2);
	camera2->SetOrthographic(true);
	camera2->SetOrthographicSize(camera2->GetTargetHeight() / 2.0f);
	camera2->SetClipNear(-1);
	camera2->SetClipFar(1);

	auto canvas = GameObject::Create("canvas")->AddComponent<UICanvasRenderer>();
	canvas->GetTransform()->SetParent(camera2->GetTransform());
	canvas->SetSize(Vector2((float) camera2->GetTargetWidth(), (float) camera2->GetTargetHeight()));

	auto atlas = Atlas::Create();
	atlas->AddSprite("sprite", Sprite::Create(Rect(0, 0, 1, 1), Vector4(0, 0, 0, 0)));
	atlas->SetTexture(rt->color_texture);

	auto sprite = GameObject::Create("sprite")->AddComponent<UISprite>();
	sprite->GetTransform()->SetParent(canvas->GetTransform());
#if VR_GLES
	sprite->GetTransform()->SetLocalScale(Vector3(1, -1, 1));
#endif
	sprite->SetAtlas(atlas);
	sprite->SetSpriteName("sprite");
	sprite->SetAnchors(Vector2(0, 0), Vector2(1, 1));
	sprite->SetOffsets(Vector2(0, 0), Vector2(0, 0));
	sprite->OnAnchor();

	canvas->GetGameObject()->SetLayerRecursively(2);
#endif
}
