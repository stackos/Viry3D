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
#include "ios/ARScene.h"
#include "graphics/Graphics.h"
#include "graphics/Camera.h"
#include "graphics/Material.h"
#include "renderer/MeshRenderer.h"
#include "container/Map.h"

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
                
                for (const auto& i : anchors)
                {
                    Ref<MeshRenderer> plane;
                    
                    if (m_planes.Contains(i.id) == false)
                    {
                        plane = GameObject::Create("ground")->AddComponent<MeshRenderer>();
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
    Map<String, Ref<MeshRenderer>> m_planes;
#endif
    
    Ref<Camera> m_camera;
};

#if 0
VR_MAIN(AppAR);
#endif
