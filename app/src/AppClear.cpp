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
#include "graphics/Camera.h"

using namespace Viry3D;

class AppClear : public Application
{
public:
	AppClear()
    {
        this->SetName("Viry3D::AppClear");
        this->SetInitSize(1280, 720);
    }
    
	virtual void Start()
    {
        auto camera = GameObject::Create("camera")->AddComponent<Camera>();
        camera->SetClearColor(Color(1, 0, 0, 1));
    }
};

#if 0
VR_MAIN(AppClear);
#endif
