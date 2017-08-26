#pragma once

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
		//std::function<void()> on_drag_begin;
		//std::function<void()> on_drag;
		//std::function<void()> on_drag_end;
	};
}