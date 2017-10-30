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

#include "DisplayWindows.h"
#include "Application.h"
#include "graphics/Graphics.h"
#include "time/Time.h"
#include "Input.h"
#include "Debug.h"
#include <windowsx.h>

extern Viry3D::Vector<Viry3D::Touch> g_input_touches;
extern Viry3D::List<Viry3D::Touch> g_input_touch_buffer;
extern bool g_key_down[(int) Viry3D::KeyCode::COUNT];
extern bool g_key[(int) Viry3D::KeyCode::COUNT];
extern bool g_key_up[(int) Viry3D::KeyCode::COUNT];
extern bool g_key_held[(int) Viry3D::KeyCode::COUNT];
extern bool g_mouse_button_down[3];
extern bool g_mouse_button_up[3];
extern Viry3D::Vector3 g_mouse_position;
extern bool g_mouse_button_held[3];

namespace Viry3D
{
	static bool g_input_down = false;

	static int get_key_code(int wParam)
	{
		int key = -1;

		if (wParam >= 48 && wParam < 48 + 10)
		{
			key = (int) KeyCode::Alpha0 + wParam - 48;
		}
		else if (wParam >= 96 && wParam < 96 + 10)
		{
			key = (int) KeyCode::Keypad0 + wParam - 96;
		}
		else if (wParam >= 65 && wParam < 65 + 'z' - 'a')
		{
			key = (int) KeyCode::A + wParam - 65;
		}
		else if (wParam == VK_BACK)
		{
			key = (int) KeyCode::Backspace;
		}
		else if (wParam == VK_SPACE)
		{
			key = (int) KeyCode::Space;
		}
		else if (wParam == VK_ESCAPE)
		{
			key = (int) KeyCode::Escape;
		}
		else if (wParam == VK_RETURN)
		{
			key = (int) KeyCode::Return;
		}

		return key;
	}

	static void switch_full_screen(HWND hWnd)
	{
		static bool full_screen = false;
		static int old_style = 0;
		static RECT old_pos;

		if (!full_screen)
		{
			full_screen = true;

			old_style = GetWindowLong(hWnd, GWL_STYLE);
			GetWindowRect(hWnd, &old_pos);

			RECT rect;
			HWND desktop = GetDesktopWindow();
			GetWindowRect(desktop, &rect);
			SetWindowLong(hWnd, GWL_STYLE, WS_POPUP);
			SetWindowPos(hWnd, HWND_TOP, 0, 0, rect.right, rect.bottom, SWP_SHOWWINDOW);
		}
		else
		{
			full_screen = false;

			SetWindowLong(hWnd, GWL_STYLE, old_style);
			SetWindowPos(hWnd, HWND_TOP,
				old_pos.left,
				old_pos.top,
				old_pos.right - old_pos.left,
				old_pos.bottom - old_pos.top,
				SWP_SHOWWINDOW);
		}
	}

	LRESULT CALLBACK win_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
			case WM_CLOSE:
				PostQuitMessage(0);
				break;

			case WM_SIZE:
				if (wParam != SIZE_MINIMIZED)
				{
					int width = lParam & 0xffff;
					int height = (lParam & 0xffff0000) >> 16;

					Application::Current()->OnResize(width, height);
				}
				break;

			case WM_KEYDOWN:
			{
				int key = get_key_code((int) wParam);

				if (key >= 0)
				{
					if (!g_key_held[key])
					{
						g_key_down[key] = true;
						g_key_held[key] = true;
						g_key[key] = true;
					}
				}
			}
			break;

			case WM_KEYUP:
			{
				int key = get_key_code((int) wParam);

				if (key >= 0)
				{
					g_key_up[key] = true;
					g_key_held[key] = false;
					g_key[key] = false;
				}
			}
			break;

			case WM_SYSCHAR:
				if (wParam == VK_RETURN)
				{
					//Alt + Enter
					switch_full_screen(hWnd);
				}
				break;

			case WM_LBUTTONDOWN:
			{
				int x = GET_X_LPARAM(lParam);
				int y = GET_Y_LPARAM(lParam);

				if (!g_input_down)
				{
					Touch t;
					t.deltaPosition = Vector2(0, 0);
					t.deltaTime = 0;
					t.fingerId = 0;
					t.phase = TouchPhase::Began;
					t.position = Vector2((float) x, (float) Graphics::GetDisplay()->GetHeight() - y - 1);
					t.tapCount = 1;
					t.time = Time::GetRealTimeSinceStartup();

					if (!g_input_touches.Empty())
					{
						g_input_touch_buffer.AddLast(t);
					}
					else
					{
						g_input_touches.Add(t);
					}

					g_input_down = true;
				}

				g_mouse_button_down[0] = true;
				g_mouse_position.x = (float) x;
				g_mouse_position.y = (float) Graphics::GetDisplay()->GetHeight() - y - 1;
				g_mouse_button_held[0] = true;
			}
			break;

			case WM_RBUTTONDOWN:
			{
				int x = GET_X_LPARAM(lParam);
				int y = GET_Y_LPARAM(lParam);

				g_mouse_button_down[1] = true;
				g_mouse_position.x = (float) x;
				g_mouse_position.y = (float) Graphics::GetDisplay()->GetHeight() - y - 1;
				g_mouse_button_held[1] = true;
			}
			break;

			case WM_MBUTTONDOWN:
			{
				int x = GET_X_LPARAM(lParam);
				int y = GET_Y_LPARAM(lParam);

				g_mouse_button_down[2] = true;
				g_mouse_position.x = (float) x;
				g_mouse_position.y = (float) Graphics::GetDisplay()->GetHeight() - y - 1;
				g_mouse_button_held[2] = true;
			}
			break;

			case WM_MOUSEMOVE:
			{
				int x = GET_X_LPARAM(lParam);
				int y = GET_Y_LPARAM(lParam);

				if (g_input_down)
				{
					Touch t;
					t.deltaPosition = Vector2(0, 0);
					t.deltaTime = 0;
					t.fingerId = 0;
					t.phase = TouchPhase::Moved;
					t.position = Vector2((float) x, (float) Graphics::GetDisplay()->GetHeight() - y - 1);
					t.tapCount = 1;
					t.time = Time::GetRealTimeSinceStartup();

					if (!g_input_touches.Empty())
					{
						if (g_input_touch_buffer.Empty())
						{
							g_input_touch_buffer.AddLast(t);
						}
						else
						{
							if (g_input_touch_buffer.Last().phase == TouchPhase::Moved)
							{
								g_input_touch_buffer.Last() = t;
							}
							else
							{
								g_input_touch_buffer.AddLast(t);
							}
						}
					}
					else
					{
						g_input_touches.Add(t);
					}
				}

				g_mouse_position.x = (float) x;
				g_mouse_position.y = (float) Graphics::GetDisplay()->GetHeight() - y - 1;
			}
			break;

			case WM_LBUTTONUP:
			{
				int x = GET_X_LPARAM(lParam);
				int y = GET_Y_LPARAM(lParam);

				if (g_input_down)
				{
					Touch t;
					t.deltaPosition = Vector2(0, 0);
					t.deltaTime = 0;
					t.fingerId = 0;
					t.phase = TouchPhase::Ended;
					t.position = Vector2((float) x, (float) Graphics::GetDisplay()->GetHeight() - y - 1);
					t.tapCount = 1;
					t.time = Time::GetRealTimeSinceStartup();

					if (!g_input_touches.Empty())
					{
						g_input_touch_buffer.AddLast(t);
					}
					else
					{
						g_input_touches.Add(t);
					}

					g_input_down = false;
				}

				g_mouse_button_up[0] = true;
				g_mouse_position.x = (float) x;
				g_mouse_position.y = (float) Graphics::GetDisplay()->GetHeight() - y - 1;
				g_mouse_button_held[0] = false;
			}
			break;

			case WM_RBUTTONUP:
			{
				int x = GET_X_LPARAM(lParam);
				int y = GET_Y_LPARAM(lParam);

				g_mouse_button_up[1] = true;
				g_mouse_position.x = (float) x;
				g_mouse_position.y = (float) Graphics::GetDisplay()->GetHeight() - y - 1;
				g_mouse_button_held[1] = false;
			}
			break;

			case WM_MBUTTONUP:
			{
				int x = GET_X_LPARAM(lParam);
				int y = GET_Y_LPARAM(lParam);

				g_mouse_button_up[2] = true;
				g_mouse_position.x = (float) x;
				g_mouse_position.y = (float) Graphics::GetDisplay()->GetHeight() - y - 1;
				g_mouse_button_held[2] = false;
			}
			break;

			default:
				break;
		}

		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	DisplayWindows::DisplayWindows()
	{
		m_window = NULL;
#if VR_GLES
		m_hdc = NULL;
		m_context = NULL;
#endif
	}

	DisplayWindows::~DisplayWindows()
	{
#if VR_GLES
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(m_shared_context);
		wglDeleteContext(m_context);
		ReleaseDC(m_window, m_hdc);
#endif
	}

	void DisplayWindows::Init(int width, int height, int fps)
	{
		DisplayBase::Init(width, height, fps);

		this->CreateSystemWindow();
	}

	void DisplayWindows::CreateSystemWindow()
	{
		String app_name = Application::Current()->GetName();
		assert(!app_name.Empty());
		const char* name = app_name.CString();

		HINSTANCE inst = GetModuleHandle(NULL);

		WNDCLASSEX win_class;
		ZeroMemory(&win_class, sizeof(win_class));

		win_class.cbSize = sizeof(WNDCLASSEX);
		win_class.style = CS_HREDRAW | CS_VREDRAW;
		win_class.lpfnWndProc = win_proc;
		win_class.cbClsExtra = 0;
		win_class.cbWndExtra = 0;
		win_class.hInstance = inst;
		win_class.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
		win_class.lpszMenuName = NULL;
		win_class.lpszClassName = name;
		win_class.hCursor = LoadCursor(NULL, IDC_ARROW);
		win_class.hIcon = (HICON) LoadImage(NULL, "icon.ico", IMAGE_ICON, SM_CXICON, SM_CYICON, LR_LOADFROMFILE);
		win_class.hIconSm = win_class.hIcon;

		if (!RegisterClassEx(&win_class))
		{
			return;
		}

		DWORD style = WS_OVERLAPPEDWINDOW;
		DWORD style_ex = 0;

		RECT wr = { 0, 0, m_width, m_height };
		AdjustWindowRect(&wr, style, FALSE);

		int x = (GetSystemMetrics(SM_CXSCREEN) - m_width) / 2 + wr.left;
		int y = (GetSystemMetrics(SM_CYSCREEN) - m_height) / 2 + wr.top;

		HWND hwnd = CreateWindowEx(
			style_ex,			// window ex style
			name,				// class name
			name,				// app name
			style,			    // window style
			x, y,				// x, y
			wr.right - wr.left, // width
			wr.bottom - wr.top, // height
			NULL,				// handle to parent
			NULL,               // handle to menu
			inst,				// hInstance
			NULL);              // no extra parameters
		if (!hwnd)
		{
			return;
		}

		ShowWindow(hwnd, SW_SHOW);

		m_window = hwnd;

#if VR_GLES
		m_hdc = GetDC(hwnd);

		PIXELFORMATDESCRIPTOR pfd = {
			sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd
			1,                     // version number
			PFD_DRAW_TO_WINDOW |   // support window
			PFD_SUPPORT_OPENGL |   // support OpenGL
			PFD_DOUBLEBUFFER,      // double buffered
			PFD_TYPE_RGBA,         // RGBA type
			24,                    // 24-bit color depth
			0, 0, 0, 0, 0, 0,      // color bits ignored
			0,                     // no alpha buffer
			0,                     // shift bit ignored
			0,                     // no accumulation buffer
			0, 0, 0, 0,            // accum bits ignored
			32,                    // 32-bit z-buffer
			0,                     // no stencil buffer
			0,                     // no auxiliary buffer
			PFD_MAIN_PLANE,        // main layer
			0,                     // reserved
			0, 0, 0                // layer masks ignored
		};

		int format_index = ChoosePixelFormat(m_hdc, &pfd);
		SetPixelFormat(m_hdc, format_index, &pfd);

		m_context = wglCreateContext(m_hdc);

		m_shared_context = wglCreateContext(m_hdc);
		wglShareLists(m_context, m_shared_context);

		wglMakeCurrent(m_hdc, m_context);
		wglewInit();

		wglSwapIntervalEXT(0);
#endif
	}

	void DisplayWindows::ProcessSystemEvents()
	{
		MSG msg;
		while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (WM_QUIT == msg.message)
			{
				Application::Quit();
			}
			else
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}
	}
}
