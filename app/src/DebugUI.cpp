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

#include "DebugUI.h"
#include "Resource.h"
#include "graphics/Graphics.h"
#include "ui/UISprite.h"
#include "ui/UILabel.h"
#include "ui/UICanvasRenderer.h"
#include "tweener/TweenUIColor.h"
#include "tweener/TweenPosition.h"
#include "time/Time.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(DebugUI);

	void DebugUI::DeepCopy(const Ref<Object>& source)
	{
		Component::DeepCopy(source);
	}

	void DebugUI::OnResize(int width, int height)
	{
		if (m_ui)
		{
			auto canvas = m_ui->GetComponent<UICanvasRenderer>();
			canvas->SetSize(Vector2((float) width, (float) height));
			canvas->OnAnchor();
		}
	}

	void DebugUI::Start()
	{
		m_ui = Resource::LoadGameObject("Assets/AppUI/debug_ui.prefab");
		m_ui->GetTransform()->SetPosition(Vector3::Zero());
		m_ui->GetTransform()->SetScale(Vector3::One());

		auto button_main = m_ui->GetTransform()->Find("button main")->GetGameObject();
		auto window_menu = m_ui->GetTransform()->Find("window menu")->GetGameObject();
		auto window_fps = m_ui->GetTransform()->Find("window fps")->GetGameObject();

		m_window_menu_pos = window_menu->GetTransform()->GetLocalPosition();
		m_fps_text = window_fps->GetTransform()->Find("Text Canvas/Text")->GetGameObject()->GetComponent<UILabel>();

		{
			auto renderers = window_fps->GetComponentsInChildren<UICanvasRenderer>();
			for (auto& i : renderers)
			{
				i->SetColor(Color(1, 1, 1, 0.8f));
			}
		}

		auto button_main_border = button_main->GetTransform()->Find("border")->GetGameObject()->GetComponent<UISprite>();
		button_main_border->event_handler.enable = true;
		button_main_border->event_handler.on_pointer_click = [=](UIPointerEvent& e) {
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
			tc->to = Color(1, 1, 1, 0.8f);

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
			window_menu->SetActive(false);
		};

		auto button_fps = window_menu->GetTransform()->Find("left bar/button fps")->GetGameObject()->GetComponent<UISprite>();
		button_fps->event_handler.enable = true;
		button_fps->event_handler.on_pointer_click = [=](UIPointerEvent& e) {
			window_fps->SetActive(!window_fps->IsActiveSelf());
		};

		auto button_profiler = window_menu->GetTransform()->Find("left bar/button profiler")->GetGameObject()->GetComponent<UISprite>();
		button_profiler->event_handler.enable = true;
		button_profiler->event_handler.on_pointer_click = [=](UIPointerEvent& e) {

		};
	}

	void DebugUI::Update()
	{
		if (m_fps_text->GetGameObject()->IsActiveInHierarchy())
		{
			auto text = String::Format("W:%d H:%d DC:%d FPS:%d",
				Graphics::GetDisplay()->GetWidth(),
				Graphics::GetDisplay()->GetHeight(),
				Graphics::draw_call,
				Time::GetFPS());
			m_fps_text->SetText(text);
		}
	}
}