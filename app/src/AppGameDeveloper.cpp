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
#include "math/Mathf.h"
#include "graphics/Camera.h"
#include "graphics/RenderTexture.h"
#include "graphics/Material.h"
#include "renderer/MeshRenderer.h"
#include "AppGameDeveloper/CodeEditor.h"
#include "AppGameDeveloper/LuaRunner.h"

using namespace Viry3D;

#define LAYER_DEFAULT 0
#define LAYER_CODE_EDITOR 1
#define RENDER_DEPTH_DEFAULT 1
#define RENDER_DEPTH_CODE_EDITOR 0
#define CODE_EDITOR_WINDOW_WIDTH 1280
#define CODE_EDITOR_WINDOW_HEIGHT 720
#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900
#define CODE_EDITOR_SCREEN_MODEL_WIDTH 0.48f
#define CODE_EDITOR_SCREEN_MODEL_HEIGHT 0.27f

class AppGameDeveloper: public Application
{
public:
	AppGameDeveloper()
	{
		this->SetName("Viry3D::AppGameDeveloper");
		this->SetInitSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	}

	virtual void Start()
	{
		auto camera = GameObject::Create("Camera")->AddComponent<Camera>();
		camera->SetCullingMask(1 << LAYER_DEFAULT);
		camera->SetDepth(RENDER_DEPTH_DEFAULT);
		camera->SetClearColor(Color(45, 45, 48, 255) / 255.0f);

		String source = 
			"print(\"Hello World!\")\r\n" \
			"print(1 + 2)\r\n" \
			"function func(a, b)\r\n" \
			"    print(a + b)\r\n" \
			"end\r\n" \
			"func(3, 4)\r\n";

		CodeEditor::RegisterComponent();
		auto code_editor = GameObject::Create("CodeEditor")->AddComponent<CodeEditor>();
		code_editor->GetGameObject()->SetLayer(LAYER_CODE_EDITOR);
		code_editor->SetRenderDepth(RENDER_DEPTH_CODE_EDITOR);
		code_editor->SetTargetScreenSize(CODE_EDITOR_WINDOW_WIDTH, CODE_EDITOR_WINDOW_HEIGHT);
		code_editor->CreateCamera();
		code_editor->LoadSource(source);

		auto quad_mesh = Resource::LoadMesh("Assets/Library/unity default resources.Quad.mesh");
		auto quad_mat = Material::Create("Diffuse");
		quad_mat->SetMainTexture(code_editor->GetTargetRenderTexture());
		quad_mat->SetMainTextureST(Vector4(1, -1, 0, 1));

		float code_editor_quad_z = (CODE_EDITOR_SCREEN_MODEL_HEIGHT / 2) / (CODE_EDITOR_WINDOW_HEIGHT / (float) WINDOW_HEIGHT * tan(Mathf::Deg2Rad * camera->GetFieldOfView() / 2));

		auto code_editor_quad = GameObject::Create("CodeEditorQuad")->AddComponent<MeshRenderer>();
		code_editor_quad->SetSharedMaterial(quad_mat);
		code_editor_quad->SetSharedMesh(quad_mesh);
		code_editor_quad->GetTransform()->SetPosition(Vector3(0, 0, code_editor_quad_z));
		code_editor_quad->GetTransform()->SetScale(Vector3(CODE_EDITOR_SCREEN_MODEL_WIDTH, CODE_EDITOR_SCREEN_MODEL_HEIGHT, 1));

		LuaRunner::RegisterComponent();
		auto lua_runner = GameObject::Create("LuaRunner")->AddComponent<LuaRunner>();
		lua_runner->RunSource(source);
	}
};

#if 1
VR_MAIN(AppGameDeveloper);
#endif
