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
#include "Input.h"
#include "animation/Animation.h"
#include "renderer/MeshRenderer.h"
#include "graphics/Graphics.h"
#include "graphics/Camera.h"
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

	virtual void Start()
	{
		this->CreateFPSUI(20, 1, 1);

		auto camera = GameObject::Create("camera")->AddComponent<Camera>();
		camera->SetCullingMask(1 << 0);
		camera->GetTransform()->SetPosition(Vector3(0, 3, -5.0f));
		camera->GetTransform()->SetRotation(Quaternion::Euler(30, 0, 0));

		int shadowmap_size = 1024;

		auto shadow_rt = RefMake<FrameBuffer>();
		shadow_rt->color_texture = RenderTexture::Create(shadowmap_size, shadowmap_size, RenderTextureFormat::R8, DepthBuffer::Depth_0, FilterMode::Bilinear);
		shadow_rt->depth_texture = RenderTexture::Create(shadowmap_size, shadowmap_size, RenderTextureFormat::Depth, DepthBuffer::Depth_24, FilterMode::Bilinear);
		
		auto shadow_camera = GameObject::Create("camera")->AddComponent<Camera>();
		shadow_camera->SetOrthographic(true);
		shadow_camera->SetOrthographicSize(2);
		shadow_camera->SetClipNear(-5);
		shadow_camera->SetClipFar(5);
		shadow_camera->SetDepth(-1);
		shadow_camera->SetCullingMask(1 << 0);
		shadow_camera->GetTransform()->SetRotation(Quaternion::Euler(45, 145, 0));
		shadow_camera->SetFrameBuffer(shadow_rt);
		shadow_camera->SetRenderMode(CameraRenderMode::ShadowMap);

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

		auto plane_mat = Material::Create("Shadow/Diffuse");
		plane_mat->SetTexture("_ShadowMap", shadow_rt->depth_texture);
		plane_mat->SetMatrix("_ViewProjectionLight", shadow_camera->GetProjectionMatrix() * shadow_camera->GetViewMatrix());
		plane_mat->SetVector("_ShadowMapTexel", Vector4(1.0f / shadowmap_size, 1.0f / shadowmap_size));
		plane_mat->SetVector("_ShadowParam", Vector4(shadow_bias, shadow_strength));

		auto plane = GameObject::Create("plane")->AddComponent<MeshRenderer>();
		plane->SetSharedMaterial(plane_mat);
		plane->SetSharedMesh(plane_mesh);

		auto sphere_mesh = Resource::LoadMesh("Assets/Library/unity default resources.Sphere.mesh");
		sphere_mesh->Update();

		auto sphere_mat = Material::Create("Gizmos");
		sphere_mat->SetMainColor(Color(0, 1, 0, 1));

		// make weak ref avoid cycle ref
		WeakRef<Camera> camera_weak = camera;
		camera->SetPostRenderFunc([=]() {
			if (m_mouse_down)
			{
				auto pos = Input::GetMousePosition();
				auto ray = camera_weak.lock()->ScreenPointToRay(pos);
				auto plane_point = plane->GetTransform()->GetPosition();
				auto plane_normal = plane->GetTransform()->GetUp();

				float ray_length;
				if (Mathf::RayPlaneIntersection(ray, plane_normal, plane_point, ray_length))
				{
					auto point = ray.GetPoint(ray_length);

					auto sphere_matrix = Matrix4x4::TRS(point, Quaternion::Identity(), Vector3::One() * 0.1f);
					Graphics::DrawMesh(sphere_mesh, sphere_matrix, sphere_mat);
				}
			}

			Viry3D::Rect rect(0.8f, 0.8f, 0.2f, 0.2f);
			Graphics::DrawQuad(&rect, shadow_rt->depth_texture, true);
		});
	}

	virtual void Update()
	{
		if (Input::GetMouseButtonDown(0))
		{
			m_mouse_down = true;
		}
		else if (Input::GetMouseButtonUp(0))
		{
			m_mouse_down = false;
		}
	}

	bool m_mouse_down = false;
};

#if 0
VR_MAIN(AppShadow);
#endif
