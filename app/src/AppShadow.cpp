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
#include "graphics/Graphics.h"
#include "graphics/Camera.h"
#include "animation/Animation.h"
#include "renderer/MeshRenderer.h"
#include "graphics/Material.h"
#include "graphics/RenderTexture.h"

using namespace Viry3D;

class AppShadow : public Application
{
public:
	AppShadow()
	{
		this->SetName("Viry3D::AppShadow");
		this->SetInitSize(1280, 720);
	}

	virtual ~AppShadow()
	{
		//Graphics::GetDisplay()->EndRecord();
	}

	virtual void Start()
	{
		this->CreateFPSUI(20, 1, 1);

		auto camera = GameObject::Create("camera")->AddComponent<Camera>();
		camera->SetClearColor(Color(1, 0, 0, 1));
		camera->SetCullingMask(1 << 0);
		camera->GetTransform()->SetPosition(Vector3(0, 3, -5.0f));
		camera->GetTransform()->SetRotation(Quaternion::Euler(30, 0, 0));

		int shadowmap_size = 1024;

		auto rt_shadow = RefMake<FrameBuffer>();
		rt_shadow->color_texture = RenderTexture::Create(shadowmap_size, shadowmap_size, RenderTextureFormat::R8, DepthBuffer::Depth_0, FilterMode::Bilinear);
		rt_shadow->depth_texture = RenderTexture::Create(shadowmap_size, shadowmap_size, RenderTextureFormat::Depth, DepthBuffer::Depth_24, FilterMode::Bilinear);
		
		auto camera_shadow = GameObject::Create("camera")->AddComponent<Camera>();
		camera_shadow->SetOrthographic(true);
		camera_shadow->SetOrthographicSize(2);
		camera_shadow->SetClipNear(-5);
		camera_shadow->SetClipFar(5);
		camera_shadow->SetDepth(-1);
		camera_shadow->SetCullingMask(1 << 0);
		camera_shadow->GetTransform()->SetRotation(Quaternion::Euler(45, 145, 0));
		camera_shadow->SetFrameBuffer(rt_shadow);
		camera_shadow->SetRenderMode(CameraRenderMode::ShadowMap);

		auto anim_obj = Resource::LoadGameObject("Assets/AppAnim/unitychan.prefab");
		anim_obj->GetTransform()->SetRotation(Quaternion::Euler(0, 180, 0));
		auto anim = anim_obj->GetComponent<Animation>();
		auto anim_state = anim->GetAnimationState("WAIT03");
		anim_state.wrap_mode = AnimationWrapMode::Loop;
		anim->UpdateAnimationState("WAIT03", anim_state);
		anim->Play("WAIT03");

		auto plane_mesh = Resource::LoadMesh("Assets/Library/unity default resources.Plane.mesh");
		plane_mesh->Update();

		float shadow_bias = 0.005f;
		float shadow_strength = 0.7f;

		auto ground_mat = Material::Create("Custom/AR/ShadowReciever");
		ground_mat->SetTexture("_ShadowMap", rt_shadow->depth_texture);
		ground_mat->SetMatrix("_ViewProjectionLight", camera_shadow->GetViewProjectionMatrix());
		ground_mat->SetVector("_ShadowMapTexel", Vector4(1.0f / shadowmap_size, 1.0f / shadowmap_size));
		ground_mat->SetVector("_ShadowParam", Vector4(shadow_bias, shadow_strength));

		auto ground = GameObject::Create("ground")->AddComponent<MeshRenderer>();
		ground->SetSharedMaterial(ground_mat);
		ground->SetSharedMesh(plane_mesh);

		camera->SetPostRenderFunc([=]() {
			Viry3D::Rect rect(0.8f, 0.8f, 0.2f, 0.2f);
			Graphics::DrawQuad(&rect, rt_shadow->depth_texture, true);
		});

		//Graphics::GetDisplay()->BeginRecord("../../../demo.mp4");
	}
};

#if 1
VR_MAIN(AppShadow);
#endif
