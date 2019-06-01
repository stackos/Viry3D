/*
* Viry3D
* Copyright 2014-2019 by Stack - stackos@qq.com
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

#pragma once

#include "Component.h"
#include "graphics/Camera.h"
#include "graphics/MeshRenderer.h"
#include "graphics/Skybox.h"
#include "animation/Animation.h"
#include "Engine.h"
#include "Resources.h"
#include "Input.h"

namespace Viry3D
{
	class App : public Component
	{
	public:
		Camera* m_camera = nullptr;
		GameObject* m_skin = nullptr;
		Vector2 m_last_touch_pos;
		Vector3 m_camera_rot = Vector3(5, 180, 0);

		App()
		{
			auto cylinder = Resources::LoadMesh("Library/unity default resources.Cylinder.mesh");
			auto texture = Resources::LoadTexture("texture/checkflag.png.tex");
			auto cubemap = Resources::LoadTexture("texture/env/sunny.tex");

			auto material = RefMake<Material>(Shader::Find("Unlit/Texture"));
			material->SetTexture(MaterialProperty::TEXTURE, texture);
			material->SetVector(MaterialProperty::TEXTURE_SCALE_OFFSET, Vector4(20, 20, 0, 0));

			auto color = Texture::CreateRenderTexture(
				1280,
				720,
				TextureFormat::R8G8B8A8,
				FilterMode::Linear,
				SamplerAddressMode::ClampToEdge);
			auto depth = Texture::CreateRenderTexture(
				1280,
				720,
				Texture::SelectDepthFormat(),
				FilterMode::Linear,
				SamplerAddressMode::ClampToEdge);

			auto camera = GameObject::Create("")->AddComponent<Camera>();
			camera->GetTransform()->SetPosition(Vector3(0, 1, 3));
			camera->GetTransform()->SetRotation(Quaternion::Euler(m_camera_rot));
			camera->SetRenderTarget(color, depth);
			camera->SetCullingMask(1 << 0);
			m_camera = camera.get();

			auto skybox = GameObject::Create("")->AddComponent<Skybox>();
			skybox->SetTexture(cubemap);

			auto floor = GameObject::Create("")->AddComponent<MeshRenderer>();
			floor->GetTransform()->SetPosition(Vector3(0, -0.01f, 0));
			floor->GetTransform()->SetScale(Vector3(20, 0.02f, 20));
			floor->SetMesh(cylinder);
			floor->SetMaterial(material);

			auto obj_0 = Resources::LoadGameObject("Resources/res/animations/unitychan/unitychan.go");
			obj_0->GetTransform()->SetPosition(Vector3(0, 0, 0));
			auto anim = obj_0->GetComponent<Animation>();
			anim->Play(0);

			auto obj_1 = Resources::LoadGameObject("Resources/res/animations/unitychan/unitychan.go");
			obj_1->GetTransform()->SetPosition(Vector3(-2, 0, -2));

			auto obj_2 = Resources::LoadGameObject("Resources/res/animations/unitychan/unitychan.go");
			obj_2->GetTransform()->SetPosition(Vector3(2, 0, -2));
			m_skin = obj_2.get();

			auto blit_camera = GameObject::Create("")->AddComponent<Camera>();
			blit_camera->SetOrthographic(true);
			blit_camera->SetOrthographicSize(1);
			blit_camera->SetNearClip(-1);
			blit_camera->SetFarClip(1);
			blit_camera->SetDepth(1);
			blit_camera->SetCullingMask(1 << 1);

			Vector<Mesh::Vertex> vertices(4);
			vertices[0].vertex = Vector3(-1, 1, 0);
			vertices[1].vertex = Vector3(-1, -1, 0);
			vertices[2].vertex = Vector3(1, -1, 0);
			vertices[3].vertex = Vector3(1, 1, 0);
			vertices[0].uv = Vector2(0, 0);
			vertices[1].uv = Vector2(0, 1);
			vertices[2].uv = Vector2(1, 1);
			vertices[3].uv = Vector2(1, 0);
			Vector<unsigned int> indices = {
				0, 1, 2, 0, 2, 3
			};
			auto quad_mesh = RefMake<Mesh>(std::move(vertices), std::move(indices));

			material = RefMake<Material>(Shader::Find("Unlit/Texture"));
			material->SetTexture(MaterialProperty::TEXTURE, color);
			if (Engine::Instance()->GetBackend() == filament::backend::Backend::OPENGL)
			{
				material->SetVector(MaterialProperty::TEXTURE_SCALE_OFFSET, Vector4(1, -1, 0, 1));
			}

			auto quad = GameObject::Create("")->AddComponent<MeshRenderer>();
			quad->GetGameObject()->SetLayer(1);
			quad->GetTransform()->SetScale(Vector3(1280 / 720.0f, 1, 1));
			quad->SetMesh(quad_mesh);
			quad->SetMaterial(material);
		}

		virtual ~App()
		{
			
		}

		virtual void Update()
		{
			static float deg = 0;
			deg += 1.0f;
			if (m_skin)
			{
				m_skin->GetTransform()->SetRotation(Quaternion::Euler(0, deg, 0));
			}

			// camera control
			if (Input::GetTouchCount() > 0)
			{
				const Touch& touch = Input::GetTouch(0);
				if (touch.phase == TouchPhase::Began)
				{
					m_last_touch_pos = touch.position;
				}
				else if (touch.phase == TouchPhase::Moved)
				{
					Vector2 delta = touch.position - m_last_touch_pos;
					m_last_touch_pos = touch.position;

					m_camera_rot.y += delta.x * 0.1f;
					m_camera_rot.x += -delta.y * 0.1f;
					m_camera->GetTransform()->SetRotation(Quaternion::Euler(m_camera_rot));
				}
			}
			if (Input::GetKey(KeyCode::W))
			{
				Vector3 forward = m_camera->GetTransform()->GetForward();
				Vector3 pos = m_camera->GetTransform()->GetPosition() + forward * 0.1f;
				m_camera->GetTransform()->SetPosition(pos);
			}
			if (Input::GetKey(KeyCode::S))
			{
				Vector3 forward = m_camera->GetTransform()->GetForward();
				Vector3 pos = m_camera->GetTransform()->GetPosition() - forward * 0.1f;
				m_camera->GetTransform()->SetPosition(pos);
			}
			if (Input::GetKey(KeyCode::A))
			{
				Vector3 right = m_camera->GetTransform()->GetRight();
				Vector3 pos = m_camera->GetTransform()->GetPosition() - right * 0.1f;
				m_camera->GetTransform()->SetPosition(pos);
			}
			if (Input::GetKey(KeyCode::D))
			{
				Vector3 right = m_camera->GetTransform()->GetRight();
				Vector3 pos = m_camera->GetTransform()->GetPosition() + right * 0.1f;
				m_camera->GetTransform()->SetPosition(pos);
			}
		}
	};
}
