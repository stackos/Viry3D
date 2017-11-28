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
#include "graphics/Camera.h"
#include "graphics/Mesh.h"
#include "graphics/Material.h"
#include "graphics/Texture2D.h"
#include "renderer/MeshRenderer.h"
#include "time/Time.h"

using namespace Viry3D;

class AppMesh : public Application
{
public:
	AppMesh()
    {
        this->SetName("Viry3D::AppMesh");
        this->SetInitSize(1280, 720);
    }
    
	virtual void Start()
    {
        auto camera = GameObject::Create("camera")->AddComponent<Camera>();
        camera->GetTransform()->SetPosition(Vector3(0, 6, -10));
        camera->GetTransform()->SetRotation(Quaternion::Euler(30, 0, 0));
        camera->SetCullingMask(1 << 0);
        
        auto mesh = Mesh::Create();
        mesh->vertices.Add(Vector3(-1, 1, -1));
        mesh->vertices.Add(Vector3(-1, -1, -1));
        mesh->vertices.Add(Vector3(1, -1, -1));
        mesh->vertices.Add(Vector3(1, 1, -1));
        mesh->vertices.Add(Vector3(-1, 1, 1));
        mesh->vertices.Add(Vector3(-1, -1, 1));
        mesh->vertices.Add(Vector3(1, -1, 1));
        mesh->vertices.Add(Vector3(1, 1, 1));
        mesh->uv.Add(Vector2(0, 0));
        mesh->uv.Add(Vector2(0, 1));
        mesh->uv.Add(Vector2(1, 1));
        mesh->uv.Add(Vector2(1, 0));
        mesh->uv.Add(Vector2(1, 0));
        mesh->uv.Add(Vector2(1, 1));
        mesh->uv.Add(Vector2(0, 1));
        mesh->uv.Add(Vector2(0, 0));
        unsigned short triangles[] = {
            0, 1, 2, 0, 2, 3,
            3, 2, 6, 3, 6, 7,
            7, 6, 5, 7, 5, 4,
            4, 5, 1, 4, 1, 0,
            4, 0, 3, 4, 3, 7,
            1, 5, 6, 1, 6, 2
        };
        mesh->triangles.AddRange(triangles, 36);
        
        auto mat = Material::Create("Diffuse");
        mesh->Update();
        
        auto obj = GameObject::Create("mesh");
        auto renderer = obj->AddComponent<MeshRenderer>();
        renderer->SetSharedMesh(mesh);
        renderer->SetSharedMaterial(mat);
        
        Resource::LoadTextureAsync("Assets/AppMesh/wow.png.tex",
                                   [=] (Ref<Object> obj)
                                   {
                                       auto tex = RefCast<Texture>(obj);
                                       mat->SetMainTexture(tex);
                                   });
        
        m_cube = obj;
        m_rotate_deg = 0;
        
        Resource::LoadGameObjectAsync("Assets/AppMesh/plane.prefab");
    }
    
	virtual void Update()
    {
        m_cube->GetTransform()->SetLocalRotation(Quaternion::Euler(0, m_rotate_deg, 0));
        m_rotate_deg += 30 * Time::GetDeltaTime();
    }

	Ref<GameObject> m_cube;
	float m_rotate_deg;
};

#if 1
VR_MAIN(AppMesh);
#endif
