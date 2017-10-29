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

#pragma once

#include "Action.h"
#include "math/Vector2.h"
#include "container/List.h"
#include <functional>

namespace Viry3D
{
	class UICanvasRenderer;

	class UIPointerEvent
	{
	public:
		Vector2 position;
	};

	class UIEventHandler
	{
	public:
		static void HandleUIEvent(const List<UICanvasRenderer*>& list);

	public:
		bool enable = false;
		std::function<void(UIPointerEvent& e)> on_pointer_down;
		std::function<void(UIPointerEvent& e)> on_pointer_up;
		std::function<void(UIPointerEvent& e)> on_pointer_click;
		//std::function<void(PointerEventData& e)> on_pointer_enter;
		//std::function<void(PointerEventData& e)> on_pointer_exit;
		//Action on_drag_begin;
		//Action on_drag;
		//Action on_drag_end;
	};
}
