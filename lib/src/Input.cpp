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

#include "Input.h"
#include "container/List.h"
#include "memory/Memory.h"

Viry3D::Vector<Viry3D::Touch> g_input_touches;
Viry3D::List<Viry3D::Touch> g_input_touch_buffer;
bool g_key_down[(int) Viry3D::KeyCode::COUNT];
bool g_key[(int) Viry3D::KeyCode::COUNT];
bool g_key_up[(int) Viry3D::KeyCode::COUNT];
bool g_mouse_button_down[3];
bool g_mouse_button_up[3];
Viry3D::Vector3 g_mouse_position;
Viry3D::Vector3 g_mouse_position_scale(1, 1, 1);
Viry3D::Vector3 g_mouse_position_offset(0, 0, 0);
bool g_mouse_button_held[3];

namespace Viry3D
{
	bool Input::m_multi_touch_enabled = false;

	bool Input::GetMouseButtonDown(int index)
	{
		return g_mouse_button_down[index];
	}

	bool Input::GetMouseButton(int index)
	{
		return g_mouse_button_held[index];
	}

	bool Input::GetMouseButtonUp(int index)
	{
		return g_mouse_button_up[index];
	}

	Vector3 Input::GetMousePosition()
	{
		return Vector3(
			g_mouse_position.x * g_mouse_position_scale.x + g_mouse_position_offset.x,
			g_mouse_position.y * g_mouse_position_scale.y + g_mouse_position_offset.y,
			0);
	}

	void Input::SetMousePositionScaleOffset(const Vector3& scale, const Vector3& offset)
	{
		g_mouse_position_scale = scale;
		g_mouse_position_offset = offset;
	}

	int Input::GetTouchCount()
	{
		return g_input_touches.Size();
	}

	const Touch *Input::GetTouch(int index)
	{
		if (index >= 0 && index < g_input_touches.Size())
		{
			return &g_input_touches[index];
		}

		return NULL;
	}

	bool Input::GetKeyDown(KeyCode key)
	{
		return g_key_down[(int) key];
	}

	bool Input::GetKey(KeyCode key)
	{
		return g_key[(int) key];
	}

	bool Input::GetKeyUp(KeyCode key)
	{
		return g_key_up[(int) key];
	}

	void Input::Update()
	{
		g_input_touches.Clear();
		if (!g_input_touch_buffer.Empty())
		{
			g_input_touches.Add(g_input_touch_buffer.First());
			g_input_touch_buffer.RemoveFirst();
		}

		Memory::Zero(g_key_down, sizeof(g_key_down));
		Memory::Zero(g_key_up, sizeof(g_key_up));
		Memory::Zero(g_mouse_button_down, sizeof(g_mouse_button_down));
		Memory::Zero(g_mouse_button_up, sizeof(g_mouse_button_up));
	}

	void Input::EnableMultiTouch(bool value)
	{
	}

	void Input::ResetInputAxes()
	{
	}
}
