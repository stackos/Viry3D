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
#include "Layer.h"
#include "ios/ARScene.h"
#include "graphics/Graphics.h"
#include "graphics/Camera.h"
#include "graphics/Material.h"
#include "graphics/RenderTexture.h"
#include "graphics//Screen.h"
#include "renderer/MeshRenderer.h"
#include "container/Map.h"
#include "animation/Animation.h"
#include "physics/Physics.h"
#include "physics/BoxCollider.h"
#include "physics/MeshCollider.h"
#include "ui/UISprite.h"
#include "ui/UILabel.h"
#include "ui/UICanvasRenderer.h"

using namespace Viry3D;

class AppAR: public Application
{
public:
    AppAR()
    {
        this->SetName("Viry3D::AppAR");
        this->SetInitSize(1280, 720);
    }

    virtual void Start()
    {
		Graphics::GetDisplay()->KeepScreenOn(true);
		Graphics::SetGlobalCullFace(CullFace::Off);

#if VR_IOS
		m_plane_mesh = Resource::LoadMesh("Assets/Library/unity default resources.Cube.mesh");

        if (ARScene::IsSupported())
        {
            m_ar = RefMake<ARScene>(1, (1 << 0) | (1 << 1), 0);
            m_ar->RunSession();
            
            m_camera = m_ar->GetCamera();
        }
#endif
        
        if (!m_camera)
        {
            m_camera = GameObject::Create("camera")->AddComponent<Camera>();
			m_camera->SetDepth(1);
            m_camera->SetCullingMask((1 << 0) | (1 << 1));
        }

		this->InitUI();
    }

	void InitUI()
	{
		m_ui_camera = GameObject::Create("camera")->AddComponent<Camera>();
		m_ui_camera->SetCullingMask(1 << (int) Layer::UI);
		m_ui_camera->SetOrthographic(true);
		m_ui_camera->SetOrthographicSize(m_ui_camera->GetTargetHeight() / 2.0f);
		m_ui_camera->SetClipNear(-500);
		m_ui_camera->SetClipFar(500);
		m_ui_camera->SetClearFlags(CameraClearFlags::Nothing);
		m_ui_camera->SetDepth(2);

		String prefab;
		if (Screen::GetWidth() >= 1080 && Screen::GetHeight() >= 1080)
		{
			prefab = "Assets/AppAR/ui_1080.prefab";
		}
		else
		{
			prefab = "Assets/AppAR/ui_720.prefab";
		}

		m_ui = Resource::LoadGameObject(prefab);
		m_ui->GetTransform()->SetPosition(Vector3::Zero());
		m_ui->GetTransform()->SetScale(Vector3::One());

		m_ui->GetComponent<UICanvasRenderer>()->SetCamera(m_ui_camera);

		DoUIResize();

		auto put = m_ui->GetTransform()->Find("put/Image")->GetGameObject()->GetComponent<UISprite>();
		put->event_handler.enable = true;
		put->event_handler.on_pointer_click = [=](UIPointerEvent& e) {
			Log("click put");
		};

		auto reset = m_ui->GetTransform()->Find("reset/Image")->GetGameObject()->GetComponent<UISprite>();
		reset->event_handler.enable = true;
		reset->event_handler.on_pointer_click = [=](UIPointerEvent& e) {
			Log("click reset");
            
            if (m_anim)
            {
                m_anim_loaded = m_anim;
                m_anim_loaded->SetActive(false);
                m_anim.reset();
            }
		};
        
        m_scan = Resource::LoadGameObject("Assets/AppAR/scan.prefab");
        m_scan->SetLayerRecursively((int) Layer::UI);
        m_scan->GetTransform()->SetParent(m_ui_camera->GetTransform());
        m_scan->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
        m_scan->GetTransform()->SetLocalRotation(Quaternion::Euler(0, 0, 0));
        m_scan->GetTransform()->SetLocalScale(Vector3::One() * 100);

		auto scan_shader = Shader::Find("Custom/AR/TransparentScan");
		auto scan_renderers = m_scan->GetComponentsInChildren<Renderer>();
		for (auto& i : scan_renderers)
		{
			i->GetSharedMaterial()->SetShader(scan_shader);
		}
	}

	void DoUIResize()
	{
		if (m_ui_camera)
		{
			m_ui_camera->SetOrthographicSize(m_ui_camera->GetTargetHeight() / 2.0f);

			auto canvas = m_ui->GetComponent<UICanvasRenderer>();
			canvas->SetSize(Vector2((float) m_ui_camera->GetTargetWidth(), (float) m_ui_camera->GetTargetHeight()));
			canvas->OnAnchor();
		}
	}

	virtual void OnResize(int width, int height)
	{
		Application::OnResize(width, height);

#if VR_IOS
        if (m_ar)
        {
            m_ar->OnResize(width, height);
        }
#endif
        
		DoUIResize();
	}
    
#if VR_IOS
    virtual void Update()
    {
        if (m_ar)
        {
            m_ar->UpdateSession();
            
            const auto& anchors = m_ar->GetAnchors();
            if (anchors.Size() > 0)
            {
                if (!m_anim)
                {
                    for (const auto& i : anchors)
                    {
                        if (i.is_plane)
                        {
                            Ref<MeshRenderer> plane;
                            
                            if (m_planes.Contains(i.id) == false)
                            {
                                plane = GameObject::Create("plane")->AddComponent<MeshRenderer>();
                                plane->SetSharedMaterial(Material::Create("Diffuse"));
                                plane->SetSharedMesh(m_plane_mesh);
                                m_planes.Add(i.id, plane);
                                
                                plane->GetGameObject()->AddComponent<BoxCollider>();
                            }
                            else
                            {
                                plane = m_planes[i.id];
                            }
                            
                            auto local_to_world_matrix = i.transform * Matrix4x4::Translation(i.center) * Matrix4x4::Scaling(i.extent);
                            plane->GetTransform()->SetLocalToWorldMatrixExternal(local_to_world_matrix);
                        }
                    }
                    
                    Vector<String> removes;
                    for (const auto& i : m_planes)
                    {
                        bool exist = false;
                        
                        for (const auto& j : anchors)
                        {
                            if (i.first == j.id)
                            {
                                exist = true;
                                break;
                            }
                        }
                        
                        if (exist == false)
                        {
                            GameObject::Destroy(i.second->GetGameObject());
                            removes.Add(i.first);
                        }
                    }
                    for (const auto& i : removes)
                    {
                        m_planes.Remove(i);
                    }
                    removes.Clear();
                    
                    this->UpdateTouch();
                }
                else
                {
                    // update anim anchor pos & rot
                    for (const auto& i : anchors)
                    {
                        if (i.is_plane == false &&
                            i.id == m_anim_anchor_id)
                        {
                            m_anim->GetTransform()->SetLocalToWorldMatrixExternal(i.transform);
                            break;
                        }
                    }
                }
            }
        }
    }
    
    virtual void OnPause()
    {
        Application::OnPause();
        
        if (m_ar)
        {
            m_ar->PauseSession();
        }
    }
    
    virtual void OnResume()
    {
        Application::OnResume();
        
        if (m_ar)
        {
            m_ar->RunSession();
        }
    }
    
    void UpdateTouch()
    {
        if (m_planes.Size() > 0)
        {
            if (Input::GetMouseButtonDown(0))
            {
                auto pos = Input::GetMousePosition();
                auto ray = m_camera->ScreenPointToRay(pos);
                
                RaycastHit hit;
                if (Physics::Raycast(hit, ray.GetOrigin(), ray.GetDirection(), 1000))
                {
                    auto point = hit.point;
                    Vector3 forward = m_camera->GetTransform()->GetPosition() - point;
                    forward.y = 0;
                    if (forward.SqrMagnitude() > 0)
                    {
                        forward.Normalize();
                    }
                    else
                    {
                        forward = Vector3(0, 0, 1);
                    }
                    
                    Matrix4x4 anchor_transform;
                    PlaceModelTo(point, forward, anchor_transform);
                    
                    if (m_anim_anchor_id.Size() > 0)
                    {
                        m_ar->RemoveAnchor(m_anim_anchor_id);
                    }
                    m_anim_anchor_id = m_ar->AddAnchor(anchor_transform);
                }
                
                if (m_anim)
                {
                    DestroyPlanes();
                }
            }
        }
    }
    
    Ref<ARScene> m_ar;
    Ref<Mesh> m_plane_mesh;
#endif
    
    void PlaceModelTo(const Vector3& pos, const Vector3& forward, Matrix4x4& transform)
    {
        if (m_anim_loaded)
        {
            m_anim = m_anim_loaded;
            m_anim->SetActive(true);
            m_anim_loaded.reset();
            
            m_anim->GetTransform()->SetPosition(pos + Vector3(0, 0, 0));
            m_anim->GetTransform()->SetScale(Vector3::One() * 0.2f);
            m_anim->GetTransform()->SetForward(forward);
            transform = m_anim->GetTransform()->GetLocalToWorldMatrix();
            
            m_shadow_plane_mat->SetMatrix("_ViewProjectionLight", m_shadow_camera->GetProjectionMatrix() * m_shadow_camera->GetViewMatrix());
        }
        else
        {
            auto anim_obj = Resource::LoadGameObject("Assets/AppAnim/unitychan.prefab");
            anim_obj->SetLayerRecursively(1);
            anim_obj->GetTransform()->SetPosition(pos + Vector3(0, 0, 0));
            anim_obj->GetTransform()->SetScale(Vector3::One() * 0.2f);
            anim_obj->GetTransform()->SetForward(forward);
            transform = anim_obj->GetTransform()->GetLocalToWorldMatrix();

            auto anim = anim_obj->GetComponent<Animation>();
            auto anim_state = anim->GetAnimationState("WAIT03");
            anim_state.wrap_mode = AnimationWrapMode::Loop;
            anim->UpdateAnimationState("WAIT03", anim_state);
            anim->Play("WAIT03");
            
            m_anim = anim_obj;
            
            int shadowmap_size = 1024;
            
            auto shadow_rt = RefMake<FrameBuffer>();
            shadow_rt->color_texture = RenderTexture::Create(shadowmap_size, shadowmap_size, RenderTextureFormat::R8, DepthBuffer::Depth_0, FilterMode::Bilinear);
            shadow_rt->depth_texture = RenderTexture::Create(shadowmap_size, shadowmap_size, RenderTextureFormat::Depth, DepthBuffer::Depth_24, FilterMode::Bilinear);
            
            auto shadow_camera = GameObject::Create("camera")->AddComponent<Camera>();
            shadow_camera->GetTransform()->SetParent(anim_obj->GetTransform());
            shadow_camera->GetTransform()->SetLocalPosition(Vector3::Zero());
            shadow_camera->GetTransform()->SetLocalRotation(Quaternion::Euler(45, 0, 0));
            shadow_camera->GetTransform()->SetLocalScale(Vector3::One());
            shadow_camera->SetOrthographic(true);
            shadow_camera->SetOrthographicSize(0.4f);
            shadow_camera->SetClipNear(-1);
            shadow_camera->SetClipFar(1);
            shadow_camera->SetCullingMask(1 << 1);
            shadow_camera->SetDepth(0);
            shadow_camera->SetFrameBuffer(shadow_rt);
            shadow_camera->SetRenderMode(CameraRenderMode::ShadowMap);
            m_shadow_camera = shadow_camera;
            
            auto plane_mesh = Resource::LoadMesh("Assets/Library/unity default resources.Plane.mesh");
            
            float shadow_bias = 0.005f;
            float shadow_strength = 0.7f;
            
            auto plane_mat = Material::Create("Custom/AR/ShadowReciever");
            plane_mat->SetTexture("_ShadowMap", shadow_rt->depth_texture);
            plane_mat->SetMatrix("_ViewProjectionLight", shadow_camera->GetProjectionMatrix() * shadow_camera->GetViewMatrix());
            plane_mat->SetVector("_ShadowMapTexel", Vector4(1.0f / shadowmap_size, 1.0f / shadowmap_size));
            plane_mat->SetVector("_ShadowParam", Vector4(shadow_bias, shadow_strength));
            m_shadow_plane_mat = plane_mat;
            
            auto plane = GameObject::Create("plane")->AddComponent<MeshRenderer>();
            plane->GetGameObject()->SetLayerRecursively(1);
            plane->GetTransform()->SetParent(anim_obj->GetTransform());
            plane->GetTransform()->SetLocalPosition(Vector3::Zero());
            plane->GetTransform()->SetLocalRotation(Quaternion::Identity());
            plane->GetTransform()->SetLocalScale(Vector3::One());
            plane->SetSharedMaterial(plane_mat);
            plane->SetSharedMesh(plane_mesh);
        }
    }
    
    void DestroyPlanes()
    {
        for (const auto& i : m_planes)
        {
            GameObject::Destroy(i.second->GetGameObject());
        }
        m_planes.Clear();
    }
    
    Ref<Camera> m_camera;
    Map<String, Ref<MeshRenderer>> m_planes;
    Ref<GameObject> m_anim;
    Ref<GameObject> m_anim_loaded;
    Ref<Material> m_shadow_plane_mat;
    Ref<Camera> m_shadow_camera;
    String m_anim_anchor_id;
	Ref<GameObject> m_scan;
	Ref<GameObject> m_ui;
	Ref<Camera> m_ui_camera;
};

#if 1
VR_MAIN(AppAR);
#endif
