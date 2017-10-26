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
		camera->SetCullingMask(1 << 0);
		camera->GetTransform()->SetPosition(Vector3(0, 3, -5.0f));
		camera->GetTransform()->SetRotation(Quaternion::Euler(30, 0, 0));

		int shadowmap_size = 1024;

		auto rt = RefMake<FrameBuffer>();
		rt->color_texture = RenderTexture::Create(shadowmap_size, shadowmap_size, RenderTextureFormat::R8, DepthBuffer::Depth_0, FilterMode::Bilinear);
		rt->depth_texture = RenderTexture::Create(shadowmap_size, shadowmap_size, RenderTextureFormat::Depth, DepthBuffer::Depth_24, FilterMode::Bilinear);
		
		auto camera_shadow = GameObject::Create("camera")->AddComponent<Camera>();
		camera_shadow->SetOrthographic(true);
		camera_shadow->SetOrthographicSize(2);
		camera_shadow->SetClipNear(-5);
		camera_shadow->SetClipFar(5);
		camera_shadow->SetDepth(-1);
		camera_shadow->SetCullingMask(1 << 0);
		camera_shadow->GetTransform()->SetRotation(Quaternion::Euler(45, 145, 0));
		camera_shadow->SetFrameBuffer(rt);
		camera_shadow->SetRenderMode(CameraRenderMode::ShadowMap);

		auto obj = Resource::LoadGameObject("Assets/AppAnim/unitychan.prefab");
		obj->GetTransform()->SetRotation(Quaternion::Euler(0, 180, 0));
		auto anim = obj->GetComponent<Animation>();
		auto state = anim->GetAnimationState("WAIT03");
		state.wrap_mode = AnimationWrapMode::Loop;
		anim->UpdateAnimationState("WAIT03", state);
		anim->Play("WAIT03");

		auto mesh = Resource::LoadMesh("Assets/Library/unity default resources.Plane.mesh");
		mesh->Update();

		float shadow_bias = 0.005f;
		float shadow_strength = 0.7f;

		auto mat = Material::Create("Shadow/Diffuse");
		mat->SetTexture("_ShadowMap", rt->depth_texture);
		mat->SetMatrix("_ViewProjectionLight", camera_shadow->GetViewProjectionMatrix());
		mat->SetVector("_ShadowMapTexel", Vector4(1.0f / shadowmap_size, 1.0f / shadowmap_size));
		mat->SetVector("_ShadowParam", Vector4(shadow_bias, shadow_strength));

		auto ground = GameObject::Create("ground")->AddComponent<MeshRenderer>();
		ground->SetSharedMaterial(mat);
		ground->SetSharedMesh(mesh);

		camera->SetPostRenderFunc([=]() {
			Viry3D::Rect rect(0.8f, 0.8f, 0.2f, 0.2f);
			Graphics::DrawQuad(&rect, rt->depth_texture, true);
		});

		//Graphics::GetDisplay()->BeginRecord("../../../demo.mp4");
	}
};

#if 1
VR_MAIN(AppShadow);
#endif
