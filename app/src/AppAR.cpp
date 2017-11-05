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
#include "ios/ARScene.h"
#include "graphics/Graphics.h"
#include "graphics/Camera.h"
#include "graphics/Material.h"
#include "graphics/RenderTexture.h"
#include "renderer/MeshRenderer.h"
#include "container/Map.h"
#include "animation/Animation.h"

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

        this->CreateFPSUI(20, 10, 10);
        
#if VR_IOS
		m_plane_mesh = Resource::LoadMesh("Assets/Library/unity default resources.Cube.mesh");
		m_plane_mesh->Update();

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
            m_camera->SetCullingMask(1 << 0);
        }
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
                            }
                            else
                            {
                                plane = m_planes[i.id];
                            }
                            
                            plane->GetTransform()->SetLocalToWorldMatrixExternal(i.transform * Matrix4x4::Translation(i.center) * Matrix4x4::Scaling(i.extent));
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
    
    virtual void OnResize(int width, int height)
    {
        Application::OnResize(width, height);
        
        if (m_ar)
        {
            m_ar->OnResize(width, height);
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
                
                for (const auto& i : m_planes)
                {
                    auto plane_point = i.second->GetTransform()->GetPosition();
                    auto plane_normal = i.second->GetTransform()->GetUp();
                    
                    float ray_length;
                    if (Mathf::RayPlaneIntersection(ray, plane_normal, plane_point, ray_length))
                    {
                        auto point = ray.GetPoint(ray_length);
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
                        m_anim_anchor_id = m_ar->AddAnchor(anchor_transform);
                        
                        break;
                    }
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
        
        auto plane_mesh = Resource::LoadMesh("Assets/Library/unity default resources.Plane.mesh");
        plane_mesh->Update();
        
        float shadow_bias = 0.005f;
        float shadow_strength = 0.7f;
        
        auto plane_mat = Material::Create("Custom/AR/ShadowReciever");
        plane_mat->SetTexture("_ShadowMap", shadow_rt->depth_texture);
        plane_mat->SetMatrix("_ViewProjectionLight", shadow_camera->GetProjectionMatrix() * shadow_camera->GetViewMatrix());
        plane_mat->SetVector("_ShadowMapTexel", Vector4(1.0f / shadowmap_size, 1.0f / shadowmap_size));
        plane_mat->SetVector("_ShadowParam", Vector4(shadow_bias, shadow_strength));
        
        auto plane = GameObject::Create("plane")->AddComponent<MeshRenderer>();
        plane->GetGameObject()->SetLayerRecursively(1);
        plane->GetTransform()->SetParent(anim_obj->GetTransform());
        plane->GetTransform()->SetLocalPosition(Vector3::Zero());
        plane->GetTransform()->SetLocalRotation(Quaternion::Identity());
        plane->GetTransform()->SetLocalScale(Vector3::One());
        plane->SetSharedMaterial(plane_mat);
        plane->SetSharedMesh(plane_mesh);
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
    String m_anim_anchor_id;
};

#if 1
VR_MAIN(AppAR);
#endif
