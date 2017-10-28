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
#include "animation/Animation.h"

using namespace Viry3D;

class AppAnim : public Application
{
public:
	AppAnim()
    {
        this->SetName("Viry3D::AppAnim");
        this->SetInitSize(1280, 720);
    }
    
	virtual void Start()
    {
        this->CreateFPSUI(20, 1, 1);
        
        auto camera = GameObject::Create("camera")->AddComponent<Camera>();
        camera->SetCullingMask(1 << 0);
        camera->GetTransform()->SetPosition(Vector3(0, 1.2f, -2.0f));
        camera->GetTransform()->SetRotation(Quaternion::Euler(10, 0, 0));
        
        auto obj = Resource::LoadGameObject("Assets/AppAnim/unitychan.prefab");
        obj->GetTransform()->SetRotation(Quaternion::Euler(0, 180, 0));
        
        auto anim = obj->GetComponent<Animation>();
        auto state = anim->GetAnimationState("WAIT03");
        state.wrap_mode = AnimationWrapMode::Loop;
        anim->UpdateAnimationState("WAIT03", state);
        anim->Play("WAIT03");
    }
};

#if 0
VR_MAIN(AppAnim);
#endif
