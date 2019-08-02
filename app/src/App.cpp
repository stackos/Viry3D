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

#include "App.h"
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
#include "Debug.h"
#include "postprocessing/Bloom.h"
#include "BoneDrawer.h"
#include "BoneMapper.h"
#include "audio/AudioManager.h"
#include "audio/AudioClip.h"
#include "audio/AudioSource.h"
#include "audio/AudioListener.h"
#include "physics/SpringBone.h"
#include "physics/SpringCollider.h"
#include "physics/SpringManager.h"
#include "CameraSwitcher.h"

namespace Viry3D
{
    class AppImplement
    {
    public:
        Camera* m_camera = nullptr;
        Camera* m_reflection_camera = nullptr;
        Camera* m_back_screen_camera = nullptr;
        Material* m_reflection_material = nullptr;
        Vector2 m_last_touch_pos;
        Vector3 m_camera_rot = Vector3(5, 180, 0);
        Label* m_fps_label = nullptr;
#if !VR_WASM
        AudioSource* m_audio_source_bgm;
#endif
        
        AppImplement()
        {
            this->InitScene();
            this->InitAudio();
            this->InitUI();
            
#if 0
            auto blit_camera = GameObject::Create("")->AddComponent<Camera>();
            blit_camera->SetClearFlags(CameraClearFlags::Nothing);
            blit_camera->SetOrthographic(true);
            blit_camera->SetOrthographicSize(1);
            blit_camera->SetNearClip(-1);
            blit_camera->SetFarClip(1);
            blit_camera->SetDepth(4);
            blit_camera->SetCullingMask(1 << 2);
            
            auto material = RefMake<Material>(Shader::Find("Unlit/Texture"));
            material->SetTexture(MaterialProperty::TEXTURE, m_reflection_camera->GetRenderTargetColor());
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
        
        ~AppImplement()
        {
#if VR_WASM
            AudioManager::StopAudio();
#endif
        }
        
        void InitScene()
        {
            this->InitBackScreen();
            this->InitReflection();
            
            auto camera = GameObject::Create("")->AddComponent<Camera>();
            camera->GetTransform()->SetPosition(Vector3(0, 1, 3.5f));
            camera->GetTransform()->SetRotation(Quaternion::Euler(m_camera_rot));
            camera->SetNearClip(0.03f);
			camera->SetFarClip(100);
            camera->SetDepth(2);
            camera->SetCullingMask((1 << 0) | (1 << 4) | (1 << 8));
            m_camera = camera.get();
            
			auto cubemap = Resources::LoadTexture("texture/env/prefilter.tex");
            auto skybox = GameObject::Create("")->AddComponent<Skybox>();
            skybox->SetTexture(cubemap, 0.0f);
            
            auto light = GameObject::Create("")->AddComponent<Light>();
            light->GetTransform()->SetRotation(Quaternion::Euler(60, 90, 0));
            light->SetType(LightType::Directional);

			auto stage = Resources::LoadGameObject("Resources/res/model/CandyRockStar/Stage/Stage Objects Group.go");
            
            auto visualizer = Resources::LoadGameObject("Resources/res/model/CandyRockStar/Visualizer/Visualizer.go");
            auto material = visualizer->GetComponent<MeshRenderer>()->GetMaterial();
            material->SetTexture("u_reflection_texture", m_reflection_camera->GetRenderTargetColor());
            material->SetTexture("u_reflection_depth_texture", m_reflection_camera->GetRenderTargetDepth());
            material->SetFloat("_ReflectionStrength", 0.2f);
            m_reflection_material = material.get();
            
            auto model = Resources::LoadGameObject("Resources/res/model/CandyRockStar/CandyRockStar.go");
            model->GetComponent<SpringManager>()->Init();
            
            this->InitBoneMapper(model);
            
            auto switcher = m_back_screen_camera->GetGameObject()->AddComponent<CameraSwitcher>();
            switcher->target = model->GetTransform()->Find("Character1_Reference/Character1_Hips/Character1_Spine/Character1_Spine1/Character1_Spine2/Character1_Neck/Character1_Head");
            switcher->points.Add(Vector3(2.348034f, 0.609365f, 1.529749f));
            switcher->points.Add(Vector3(-2.389755f, 0.609365f, 1.529749f));
            switcher->interval = 10;
            switcher->stability = 1;
            switcher->fov_curve = AnimationCurve::Linear(1, 4.962269f, 10, 4.962269f);
            switcher->Init();

			auto bloom = camera->GetGameObject()->AddComponent<Bloom>();
			bloom->SetIntensity(21.7f);
			bloom->SetThreshold(0.9f);
			bloom->SetSoftKnee(0.154f);
			bloom->SetDiffusion(3.93f);
        }
        
        void InitBoneMapper(const Ref<GameObject>& model)
        {
            auto clip = Resources::LoadGameObject("Resources/res/model/CandyRockStar/Animations/Anim_NOT01.go");
            clip->GetTransform()->SetPosition(Vector3(0, 0, 0));
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
            
            /*
            auto bone_drawer = clip->AddComponent<BoneDrawer>();
            bone_drawer->root = clip->GetTransform()->Find("Character1_Reference/Character1_Hips");
            bone_drawer->Init();
            */
        }
        
        void InitBackScreen()
        {
            const int texture_size = 1024;
            auto color_texture = Texture::CreateRenderTexture(
                texture_size,
                texture_size,
                TextureFormat::R8G8B8A8,
                FilterMode::Linear,
                SamplerAddressMode::ClampToEdge);
            auto depth_texture = Texture::CreateRenderTexture(
                texture_size,
                texture_size,
                Texture::SelectDepthFormat(),
                FilterMode::Linear,
                SamplerAddressMode::ClampToEdge);
            
            auto camera = GameObject::Create("")->AddComponent<Camera>();
            camera->GetTransform()->SetPosition(Vector3(0, 1, 1));
            camera->GetTransform()->SetRotation(Quaternion::Euler(Vector3(0, 180, 0)));
            camera->SetNearClip(0.03f);
            camera->SetDepth(0);
            camera->SetCullingMask((1 << 0));
            camera->SetRenderTarget(color_texture, depth_texture);
            camera->SetAspect(336.0f / 300.0f);
            m_back_screen_camera = camera.get();
            
            auto back_screen = Resources::LoadGameObject("Resources/res/model/CandyRockStar/Stage/Back Screen/Back Screen.go");
            auto material = back_screen->GetComponent<MeshRenderer>()->GetMaterial();
            material->SetTexture(MaterialProperty::TEXTURE, color_texture);
        }
        
        void InitReflection()
        {
            const int texture_size = 1024;
            auto color_texture = Texture::CreateRenderTexture(
                texture_size,
                texture_size,
                TextureFormat::R8G8B8A8,
                FilterMode::Linear,
                SamplerAddressMode::ClampToEdge);
            
            auto camera = GameObject::Create("")->AddComponent<Camera>();
            camera->GetTransform()->SetPosition(Vector3(0, -1, 3.5f));
            camera->GetTransform()->SetRotation(Quaternion::Euler(Vector3(-m_camera_rot.x, m_camera_rot.y, 0)));
            camera->SetNearClip(0.03f);
            camera->SetDepth(1);
            camera->SetCullingMask((1 << 0) | (1 << 8));
            camera->SetRenderTarget(color_texture, m_back_screen_camera->GetRenderTargetDepth());
            m_reflection_camera = camera.get();
        }
        
        void InitAudio()
        {
            auto listener = AudioManager::GetListener();
            listener->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
            listener->GetTransform()->SetLocalRotation(Quaternion::Euler(0, 0, 0));
            
#if !VR_WASM
            auto audio_path = "Resources/res/model/CandyRockStar/Unite In The Sky (full).mp3";
            auto audio_clip = AudioClip::LoadMp3FromFile(Engine::Instance()->GetDataPath() + "/" + audio_path);
            m_audio_source_bgm = GameObject::Create("")->AddComponent<AudioSource>().get();
            m_audio_source_bgm->SetClip(audio_clip);
            m_audio_source_bgm->SetLoop(false);
            m_audio_source_bgm->Play();
#else
            AudioManager::PlayAudio(audio_path, false);
#endif
        }
        
        void InitUI()
        {
            auto ui_camera = GameObject::Create("")->AddComponent<Camera>();
            ui_camera->SetClearFlags(CameraClearFlags::Nothing);
            ui_camera->SetDepth(3);
            ui_camera->SetCullingMask(1 << 1);
            
            auto canvas = GameObject::Create("")->AddComponent<CanvasRenderer>(FilterMode::Linear);
            canvas->GetGameObject()->SetLayer(1);
            canvas->SetCamera(ui_camera);
            
            auto label = RefMake<Label>();
            label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetPivot(Vector2(0, 0));
            label->SetColor(Color(1, 1, 1, 1));
            label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);
            canvas->AddView(label);
            m_fps_label = label.get();
        }
        
        static float sgn(float a)
        {
            if (a > 0.0f) return 1.0f;
            if (a < 0.0f) return -1.0f;
            return 0.0f;
        }
        
        static void CalculateObliqueMatrix(Matrix4x4& projection, const Vector4& clip_plane)
        {
            Vector4 q = projection.Inverse() * Vector4(sgn(clip_plane.x), sgn(clip_plane.y), 1.0f, 1.0f);
            Vector4 c = clip_plane * (2.0f / (Vector4::Dot(clip_plane, q)));
            projection.m20 = c.x - projection.m30;
            projection.m21 = c.y - projection.m31;
            projection.m22 = c.z - projection.m32;
            projection.m23 = c.w - projection.m33;
        }
        
        void Update()
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
                    if (m_reflection_camera)
                    {
                        m_reflection_camera->GetTransform()->SetRotation(Quaternion::Euler(Vector3(-m_camera_rot.x, m_camera_rot.y, 0)));
                    }
                }
            }
            if (Input::GetKey(KeyCode::W))
            {
                Vector3 forward = m_camera->GetTransform()->GetForward();
                Vector3 pos = m_camera->GetTransform()->GetPosition() + forward * 0.1f;
                m_camera->GetTransform()->SetPosition(pos);
                if (m_reflection_camera)
                {
                    m_reflection_camera->GetTransform()->SetPosition(Vector3(pos.x, -pos.y, pos.z));
                }
            }
            if (Input::GetKey(KeyCode::S))
            {
                Vector3 forward = m_camera->GetTransform()->GetForward();
                Vector3 pos = m_camera->GetTransform()->GetPosition() - forward * 0.1f;
                m_camera->GetTransform()->SetPosition(pos);
                if (m_reflection_camera)
                {
                    m_reflection_camera->GetTransform()->SetPosition(Vector3(pos.x, -pos.y, pos.z));
                }
            }
            if (Input::GetKey(KeyCode::A))
            {
                Vector3 right = m_camera->GetTransform()->GetRight();
                Vector3 pos = m_camera->GetTransform()->GetPosition() - right * 0.1f;
                m_camera->GetTransform()->SetPosition(pos);
                if (m_reflection_camera)
                {
                    m_reflection_camera->GetTransform()->SetPosition(Vector3(pos.x, -pos.y, pos.z));
                }
            }
            if (Input::GetKey(KeyCode::D))
            {
                Vector3 right = m_camera->GetTransform()->GetRight();
                Vector3 pos = m_camera->GetTransform()->GetPosition() + right * 0.1f;
                m_camera->GetTransform()->SetPosition(pos);
                if (m_reflection_camera)
                {
                    m_reflection_camera->GetTransform()->SetPosition(Vector3(pos.x, -pos.y, pos.z));
                }
            }
            if (Input::GetKey(KeyCode::Q))
            {
                Vector3 up = m_camera->GetTransform()->GetUp();
                Vector3 pos = m_camera->GetTransform()->GetPosition() + up * 0.1f;
                m_camera->GetTransform()->SetPosition(pos);
                if (m_reflection_camera)
                {
                    m_reflection_camera->GetTransform()->SetPosition(Vector3(pos.x, -pos.y, pos.z));
                }
            }
            if (Input::GetKey(KeyCode::E))
            {
                Vector3 up = m_camera->GetTransform()->GetUp();
                Vector3 pos = m_camera->GetTransform()->GetPosition() - up * 0.1f;
                m_camera->GetTransform()->SetPosition(pos);
                if (m_reflection_camera)
                {
                    m_reflection_camera->GetTransform()->SetPosition(Vector3(pos.x, -pos.y, pos.z));
                }
            }
            
            if (m_reflection_material)
            {
                auto vp = m_camera->GetProjectionMatrix() * m_camera->GetViewMatrix();
                auto vp_inverse = vp.Inverse();
                m_reflection_material->SetMatrix("_ViewProjectInverse", vp_inverse);
            }
            if (m_reflection_camera)
            {
                m_reflection_camera->SetAspect(m_camera->GetAspect());
            }
            
            /*
            // oblique projection
            auto clip_plane_pos = m_reflection_camera->GetViewMatrix().MultiplyPoint(Vector3(0, 0, 0));
            auto clip_plane_normal = m_reflection_camera->GetViewMatrix().MultiplyDirection(Vector3(0, 1, 0));
            auto clip_plane = Vector4(clip_plane_normal.x, clip_plane_normal.y, clip_plane_normal.z, -Vector3::Dot(clip_plane_pos, clip_plane_normal));
            auto oblique_projection = m_camera->GetProjectionMatrix();
            CalculateObliqueMatrix(oblique_projection, clip_plane);
            m_reflection_camera->SetProjectionMatrixExternal(oblique_projection);
            */
        }
    };
    
    App::App()
    {
        m_implement = RefMake<AppImplement>();
    }
    
    void App::Update()
    {
        m_implement->Update();
    }
}
