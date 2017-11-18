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
#include "ui/UISprite.h"
#include "ui/UICanvasRenderer.h"
#include "tweener/TweenUIColor.h"
#include "tweener/TweenPosition.h"

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
		camera->SetOrthographic(true);
		camera->SetOrthographicSize(camera->GetTargetHeight() / 2.0f);
		camera->SetClipNear(-1);
		camera->SetClipFar(1);
		camera->SetClearColor(Color::White());

		auto ui = Resource::LoadGameObject("Assets/AppUI/debug_ui.prefab");
		ui->GetTransform()->SetPosition(Vector3::Zero());
		ui->GetTransform()->SetScale(Vector3::One());

		auto button_main = ui->GetTransform()->Find("button main")->GetGameObject();
		auto window_menu = ui->GetTransform()->Find("window menu")->GetGameObject();
		m_window_menu_pos = window_menu->GetTransform()->GetLocalPosition();

		auto button_main_border = button_main->GetTransform()->Find("border")->GetGameObject()->GetComponent<UISprite>();
		button_main_border->event_handler.enable = true;
		button_main_border->event_handler.on_pointer_click = [=](UIPointerEvent& e) {
			button_main->SetActive(false);
			window_menu->SetActive(true);

			// tween color in
			auto renderers = window_menu->GetComponentsInChildren<UICanvasRenderer>();
			for (auto& i : renderers)
			{
				i->SetColor(Color(1, 1, 1, 0));
			}

			auto tc = window_menu->AddComponent<TweenUIColor>();
			tc->duration = 0.2f;
			tc->from = Color(1, 1, 1, 0);
			tc->to = Color(1, 1, 1, 0.9f);

			// tween pos in
			auto start_pos = m_window_menu_pos + Vector3(-10, 0, 0);
			window_menu->GetTransform()->SetLocalPosition(start_pos);
			auto tp = window_menu->AddComponent<TweenPosition>();
			tp->duration = 0.2f;
			tp->from = start_pos;
			tp->to = m_window_menu_pos;
		};

		// block event with background
		auto window_menu_border = window_menu->GetTransform()->Find("left bar/border")->GetGameObject()->GetComponent<UISprite>();
		window_menu_border->event_handler.enable = true;

		auto window_menu_closer = window_menu->GetTransform()->Find("closer")->GetGameObject()->GetComponent<UIView>();
		window_menu_closer->event_handler.enable = true;
		window_menu_closer->event_handler.on_pointer_click = [=](UIPointerEvent& e) {
			button_main->SetActive(true);
			window_menu->SetActive(false);
		};

		auto button_fps = window_menu->GetTransform()->Find("left bar/button fps")->GetGameObject()->GetComponent<UISprite>();
		button_fps->event_handler.enable = true;
		button_fps->event_handler.on_pointer_click = [=](UIPointerEvent& e) {

		};

		auto button_profiler = window_menu->GetTransform()->Find("left bar/button profiler")->GetGameObject()->GetComponent<UISprite>();
		button_profiler->event_handler.enable = true;
		button_profiler->event_handler.on_pointer_click = [=](UIPointerEvent& e) {

		};
    }

	Vector3 m_window_menu_pos;
};

#if 1
VR_MAIN(AppUI);
#endif
