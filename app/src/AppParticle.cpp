/*
* Viry3D
* Copyright 2014-2018 by Stack - stackos@qq.com
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

using namespace Viry3D;

class AppParticle : public Application
{
public:
	AppParticle()
    {
        this->SetName("Viry3D::AppParticle");
        this->SetInitSize(1280, 720);
    }
    
	virtual void Start()
    {
        auto camera = GameObject::Create("camera")->AddComponent<Camera>();
		camera->GetTransform()->SetPosition(Vector3(0, 0, -2000));
        camera->SetClipFar(3000);

		Resource::LoadGameObject("Assets/AppParticle/particles.prefab");
    }
};

#if 0
VR_MAIN(AppParticle);
#endif
