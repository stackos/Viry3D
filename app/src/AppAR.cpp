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
        
        this->CreateFPSUI(20, 1, 1);
        
        m_camera = GameObject::Create("camera")->AddComponent<Camera>();
        m_camera->SetCullingMask(1 << 0);
        m_camera->SetFrustumCulling(false);
        
#if VR_IOS
		m_plane_mesh = Resource::LoadMesh("Assets/Library/unity default resources.Cube.mesh");
		m_plane_mesh->Update();

        if (ARScene::IsSupported())
        {
            m_ar = RefMake<ARScene>();
            m_ar->RunSession();
        }
#endif
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
                m_camera->SetViewMatrixExternal(m_ar->GetCameraViewMatrix());
                m_camera->SetProjectionMatrixExternal(m_ar->GetCameraProjectionMatrix());
                m_camera->GetTransform()->SetLocalToWorldMatrixExternal(m_ar->GetCameraTransform());
                
                for (const auto& i : anchors)
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
    
    Ref<ARScene> m_ar;
    Ref<Mesh> m_plane_mesh;
#else
    virtual void Update()
    {
        this->UpdateTouch();
    }
#endif
    
    void UpdateTouch()
    {
        if (m_planes.Size() > 0)
        {
            if (Input::GetMouseButtonDown(0) && !m_anim)
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
                        
                        PlaceModelTo(point, forward);
					}
				}
            }
        }
    }
    
    void PlaceModelTo(const Vector3& pos, const Vector3& forward)
    {
        auto anim_obj = Resource::LoadGameObject("Assets/AppAnim/unitychan.prefab");
        anim_obj->GetTransform()->SetPosition(pos + Vector3(0, 0, 0));
        anim_obj->GetTransform()->SetScale(Vector3::One() * 0.2f);
        anim_obj->GetTransform()->SetForward(forward);
        auto anim = anim_obj->GetComponent<Animation>();
        auto anim_state = anim->GetAnimationState("WAIT03");
        anim_state.wrap_mode = AnimationWrapMode::Loop;
        anim->UpdateAnimationState("WAIT03", anim_state);
        anim->Play("WAIT03");
        
        //m_anim = anim_obj;
    }
    
    Ref<Camera> m_camera;
    Map<String, Ref<MeshRenderer>> m_planes;
    Ref<GameObject> m_anim;
};

#if 1
VR_MAIN(AppAR);
#endif
