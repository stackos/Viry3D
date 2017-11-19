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
#include "Layer.h"
#include "graphics/Camera.h"
#include "DebugUI.h"

using namespace Viry3D;

class AppUI : public Application
{
public:
	AppUI()
    {
        this->SetName("Viry3D::AppUI");
        this->SetInitSize(1280, 720);
    }
    
	virtual void Start()
    {
		auto camera = GameObject::Create("camera")->AddComponent<Camera>();
		camera->SetCullingMask(1 << (int) Layer::UI);
		camera->SetOrthographic(true);
		camera->SetOrthographicSize(camera->GetTargetHeight() / 2.0f);
		camera->SetClipNear(-1);
		camera->SetClipFar(1);
		camera->SetClearColor(Color::White());

		DebugUI::RegisterComponent();
		GameObject::Create("debug_ui")->AddComponent<DebugUI>();
    }
};

#if 0
VR_MAIN(AppUI);
#endif
