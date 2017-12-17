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
#include "graphics/Graphics.h"
#include "graphics/Camera.h"
#include "graphics/RenderTexture.h"
#include "AppGameDeveloper/CodeEditor.h"
#include "AppGameDeveloper/LuaRunner.h"

using namespace Viry3D;

#define LAYER_DEFAULT 0
#define LAYER_CODE_EDITOR 1
#define RENDER_DEPTH_DEFAULT 1
#define RENDER_DEPTH_CODE_EDITOR 0
#define CODE_EDITOR_WINDOW_WIDTH 960
#define CODE_EDITOR_WINDOW_HEIGHT 720

class AppGameDeveloper: public Application
{
public:
	AppGameDeveloper()
	{
		this->SetName("Viry3D::AppGameDeveloper");
		this->SetInitSize(1280, 720);
	}

	virtual void Start()
	{
		auto camera = GameObject::Create("Camera")->AddComponent<Camera>();
		camera->SetCullingMask(1 << LAYER_DEFAULT);
		camera->SetDepth(RENDER_DEPTH_DEFAULT);
		camera->SetClearColor(Color(45, 45, 48, 255) / 255.0f);

		CodeEditor::RegisterComponent();
		auto code_editor = GameObject::Create("CodeEditor")->AddComponent<CodeEditor>();
		code_editor->GetGameObject()->SetLayer(LAYER_CODE_EDITOR);
		code_editor->SetRenderDepth(RENDER_DEPTH_CODE_EDITOR);
		code_editor->SetTargetScreenSize(CODE_EDITOR_WINDOW_WIDTH, CODE_EDITOR_WINDOW_HEIGHT);
		code_editor->CreateCamera();

		LuaRunner::RegisterComponent();
		auto lua_runner = GameObject::Create("LuaRunner")->AddComponent<LuaRunner>();
		lua_runner->RunSource("print(\"Hello World!\")");

		camera->SetPostRenderFunc([=] () {
			float w = CODE_EDITOR_WINDOW_WIDTH / 1280.0f;
			float h = CODE_EDITOR_WINDOW_HEIGHT / 720.0f;
			float x = (1.0f - w) / 2;
			float y = (1.0f - h) / 2;
			Rect rect(x, y, w, h);
			Graphics::DrawQuad(&rect, code_editor->GetTargetRenderTexture(), false);
		});
	}
};

#if 1
VR_MAIN(AppGameDeveloper);
#endif
