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
#include "graphics/Light.h"
#include "animation/Animation.h"
#include "ui/CanvasRenderer.h"
#include "ui/Sprite.h"
#include "ui/Label.h"
#include "time/Time.h"
#include "Engine.h"
#include "Resources.h"
#include "Input.h"
#include "BoneDrawer.h"
#include "BoneMapper.h"
#include "audio/AudioManager.h"
#include "audio/AudioClip.h"
#include "audio/AudioSource.h"
#include "audio/AudioListener.h"
#include "physics/SpringCollider.h"

namespace Viry3D
{
	class App : public Component
	{
	public:
		Camera* m_camera = nullptr;
		Vector2 m_last_touch_pos;
		Vector3 m_camera_rot = Vector3(5, 180, 0);
		Label* m_fps_label = nullptr;
#if !VR_WASM
		AudioSource* m_audio_source_bgm;
#endif
        
		App()
		{
			auto cube = Resources::LoadMesh("Library/unity default resources.Cube.mesh");
			auto texture = Resources::LoadTexture("texture/checkflag.png.tex");
			auto cubemap = Resources::LoadTexture("texture/env/prefilter.tex");

			auto material = RefMake<Material>(Shader::Find("Diffuse"));
			material->SetTexture(MaterialProperty::TEXTURE, texture);
			material->SetVector(MaterialProperty::TEXTURE_SCALE_OFFSET, Vector4(40, 20, 0, 0));

			auto camera = GameObject::Create("")->AddComponent<Camera>();
			camera->GetTransform()->SetPosition(Vector3(0, 1, 2.3f));
			camera->GetTransform()->SetRotation(Quaternion::Euler(m_camera_rot));
			camera->SetNearClip(0.03f);
			camera->SetDepth(0);
			camera->SetCullingMask(1 << 0);
			m_camera = camera.get();

			auto skybox = GameObject::Create("")->AddComponent<Skybox>();
			skybox->SetTexture(cubemap, 0.0f);

			auto light = GameObject::Create("")->AddComponent<Light>();
			light->GetTransform()->SetPosition(Vector3(-1.332f, 3, 0));
			light->GetTransform()->SetRotation(Quaternion::Euler(60, 90, 0));
			light->SetType(LightType::Spot);
			light->SetRange(10);
			light->SetSpotAngle(45);
			light->EnableShadow(true);

			auto floor = GameObject::Create("")->AddComponent<MeshRenderer>();
			floor->GetTransform()->SetPosition(Vector3(0, -5, 0));
			floor->GetTransform()->SetScale(Vector3(20, 10, 10));
			floor->SetMesh(cube);
			floor->SetMaterial(material);
			floor->EnableRecieveShadow(true);

            //auto obj = Resources::LoadGameObject("Resources/res/animations/unitychan/unitychan.go");
			auto model = Resources::LoadGameObject("Resources/res/model/CandyRockStar/CandyRockStar.go");
			model->GetTransform()->SetPosition(Vector3(0, 0, 0));
			auto renderers = model->GetComponentsInChildren<Renderer>();
			for (int i = 0; i < renderers.Size(); ++i)
			{
				renderers[i]->EnableCastShadow(true);
			}
			
			auto clip = Resources::LoadGameObject("Resources/res/model/CandyRockStar/Animations/Anim_SAK01.go");
			clip->GetTransform()->SetPosition(Vector3(1, 0, 0));
			auto anim = clip->GetComponent<Animation>();
			if (anim)
			{
				anim->Play(0);
			}

			auto bone_mapper = clip->AddComponent<BoneMapper>();
			bone_mapper->root_src = clip->GetTransform()->Find("Character1_Reference/Character1_Hips");
			bone_mapper->root_dst = model->GetTransform()->Find("Character1_Reference/Character1_Hips");
			bone_mapper->src_base_pose_path = "Resources/res/model/CandyRockStar/t-pose-src.json";
			bone_mapper->dst_base_pose_path = "Resources/res/model/CandyRockStar/t-pose-dst.json";
			bone_mapper->bone_map_path = "Resources/res/model/CandyRockStar/bone-map.json";
			bone_mapper->Init();

			auto bone_drawer = clip->AddComponent<BoneDrawer>();
			bone_drawer->root = clip->GetTransform()->Find("Character1_Reference/Character1_Hips");
			bone_drawer->Init();
            
            struct ColliderConfig
            {
                String path;
                float radius;
            };
            Vector<ColliderConfig> colliders = {
                { "Character1_Reference/Character1_Hips", 0.12f },
                { "Character1_Reference/Character1_Hips/Character1_Spine/Character1_Spine1/Character1_Spine2/Character1_LeftShoulder/Character1_LeftArm", 0.12f },
                { "Character1_Reference/Character1_Hips/Character1_Spine/Character1_Spine1/Character1_Spine2/Character1_LeftShoulder/Character1_LeftArm/Character1_LeftForeArm", 0.1f },
                { "Character1_Reference/Character1_Hips/Character1_Spine/Character1_Spine1/Character1_Spine2/Character1_LeftShoulder/Character1_LeftArm/Character1_LeftForeArm/Character1_LeftHand", 0.05f },
                { "Character1_Reference/Character1_Hips/Character1_LeftUpLeg/Character1_LeftLeg", 0.15f },
                { "Character1_Reference/Character1_Hips/Character1_LeftUpLeg", 0.08f },
                { "Character1_Reference/Character1_Hips/Character1_Spine/Character1_Spine1/Character1_Spine2/Character1_RightShoulder/Character1_RightArm", 0.12f },
                { "Character1_Reference/Character1_Hips/Character1_Spine/Character1_Spine1/Character1_Spine2/Character1_RightShoulder/Character1_RightArm/Character1_RightForeArm", 0.1f },
                { "Character1_Reference/Character1_Hips/Character1_Spine/Character1_Spine1/Character1_Spine2/Character1_RightShoulder/Character1_RightArm/Character1_RightForeArm/Character1_RightHand", 0.05f },
                { "Character1_Reference/Character1_Hips/Character1_RightUpLeg/Character1_RightLeg", 0.15f },
                { "Character1_Reference/Character1_Hips/Character1_RightUpLeg", 0.08f },
                { "Character1_Reference/Character1_Hips/Character1_Spine/Character1_Spine1", 0.15f },
                { "Character1_Reference/Character1_Hips/Character1_Spine/Character1_Spine1/Character1_Spine2", 0.15f },
                { "Character1_Reference/Character1_Hips/Character1_Spine/Character1_Spine1/Character1_Spine2/Character1_Neck/Character1_Head/Locator_Head_Above", 0.12f },
                { "Character1_Reference/Character1_Hips/Character1_LeftUpLeg/Locator_LeftUpLeg_Middle", 0.07f },
                { "Character1_Reference/Character1_Hips/Character1_RightUpLeg/Locator_RightUpLeg_Middle", 0.07f },
            };
            for (int i = 0; i < colliders.Size(); ++i)
            {
                model->GetTransform()->Find(colliders[i].path)->GetGameObject()->AddComponent<SpringCollider>()->radius = colliders[i].radius;
            }
            
            auto listener = AudioManager::GetListener();
            listener->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
            listener->GetTransform()->SetLocalRotation(Quaternion::Euler(0, 0, 0));

#if !VR_WASM
            auto audio_path = "Resources/res/model/CandyRockStar/Unite In The Sky (full).mp3";
            auto audio_clip = AudioClip::LoadMp3FromFile(Engine::Instance()->GetDataPath() + "/" + audio_path);
            m_audio_source_bgm = GameObject::Create("")->AddComponent<AudioSource>().get();
            m_audio_source_bgm->SetClip(audio_clip);
            m_audio_source_bgm->SetLoop(true);
            m_audio_source_bgm->Play();
#else
            AudioManager::PlayAudio(audio_path, true);
#endif
            
			auto ui_camera = GameObject::Create("")->AddComponent<Camera>();
			ui_camera->SetClearFlags(CameraClearFlags::Nothing);
			ui_camera->SetDepth(1);
			ui_camera->SetCullingMask(1 << 1);

			auto canvas = GameObject::Create("")->AddComponent<CanvasRenderer>(FilterMode::Linear);
			canvas->GetGameObject()->SetLayer(1);
			canvas->SetCamera(ui_camera);

			auto label = RefMake<Label>();
			label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
			label->SetPivot(Vector2(0, 0));
			label->SetColor(Color(0, 0, 0, 1));
			label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);
			canvas->AddView(label);
			m_fps_label = label.get();
            
#if 0
			auto blit_camera = GameObject::Create("")->AddComponent<Camera>();
			blit_camera->SetClearFlags(CameraClearFlags::Nothing);
			blit_camera->SetOrthographic(true);
			blit_camera->SetOrthographicSize(1);
			blit_camera->SetNearClip(-1);
			blit_camera->SetFarClip(1);
			blit_camera->SetDepth(2);
			blit_camera->SetCullingMask(1 << 2);

			material = RefMake<Material>(Shader::Find("Unlit/Texture"));
			material->SetTexture(MaterialProperty::TEXTURE, light->GetShadowTexture());
			if (Engine::Instance()->GetBackend() == filament::backend::Backend::OPENGL)
			{
				material->SetVector(MaterialProperty::TEXTURE_SCALE_OFFSET, Vector4(1, -1, 0, 1));
			}

			auto quad = GameObject::Create("")->AddComponent<MeshRenderer>();
			quad->GetGameObject()->SetLayer(2);
			quad->GetTransform()->SetScale(Vector3(1, 1, 1));
			quad->SetMesh(Mesh::GetSharedQuadMesh());
			quad->SetMaterial(material);
#endif
		}

		virtual ~App()
		{
#if VR_WASM
            AudioManager::StopAudio();
#endif
		}

		virtual void Update()
		{
			if (m_fps_label)
			{
				m_fps_label->SetText(String::Format("FPS:%d", Time::GetFPS()));
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
			if (Input::GetKey(KeyCode::Q))
			{
				Vector3 up = m_camera->GetTransform()->GetUp();
				Vector3 pos = m_camera->GetTransform()->GetPosition() + up * 0.1f;
				m_camera->GetTransform()->SetPosition(pos);
			}
			if (Input::GetKey(KeyCode::E))
			{
				Vector3 up = m_camera->GetTransform()->GetUp();
				Vector3 pos = m_camera->GetTransform()->GetPosition() - up * 0.1f;
				m_camera->GetTransform()->SetPosition(pos);
			}
		}
	};
}
