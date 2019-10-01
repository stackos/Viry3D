/*
* Viry3D
* Copyright 2014-2019 by Stack - stackos@qq.com
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

#include "Engine.h"
#include "Input.h"
#include "time/Time.h"
#include "container/List.h"
#include "Debug.h"
#include <Windows.h>
#include <windowsx.h>

using namespace Viry3D;

extern Vector<Touch> g_input_touches;
extern List<Touch> g_input_touch_buffer;
extern bool g_key_down[(int) KeyCode::COUNT];
extern bool g_key[(int) KeyCode::COUNT];
extern bool g_key_up[(int) KeyCode::COUNT];
extern bool g_mouse_button_down[3];
extern bool g_mouse_button_up[3];
extern Vector3 g_mouse_position;
extern bool g_mouse_button_held[3];
extern float g_mouse_scroll_wheel;

static bool g_mouse_down = false;
static bool g_minimized = false;
static int g_window_width;
static int g_window_height;
static HWND g_wallpaper_win;
static NOTIFYICONDATA g_tray_id;
static HMENU g_tray_menu;
static Engine* g_engine;

#define WM_TRAY (WM_USER + 100)
#define WM_TRAY_OPEN (WM_TRAY + 1)
#define WM_TRAY_EXIT (WM_TRAY + 2)
#undef SendMessage

static int GetKeyCode(int wParam)
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
    else
    {
        switch (wParam)
        {
            case VK_CONTROL:
            {
                short state_l = ((unsigned short) GetKeyState(VK_LCONTROL)) >> 15;
                short state_r = ((unsigned short) GetKeyState(VK_RCONTROL)) >> 15;
                if (state_l)
                {
                    key = (int) KeyCode::LeftControl;
                }
                else if (state_r)
                {
                    key = (int) KeyCode::RightControl;
                }
                break;
            }
            case VK_SHIFT:
            {
                short state_l = ((unsigned short) GetKeyState(VK_LSHIFT)) >> 15;
                short state_r = ((unsigned short) GetKeyState(VK_RSHIFT)) >> 15;
                if (state_l)
                {
                    key = (int) KeyCode::LeftShift;
                }
                else if (state_r)
                {
                    key = (int) KeyCode::RightShift;
                }
                break;
            }
            case VK_MENU:
            {
                short state_l = ((unsigned short) GetKeyState(VK_LMENU)) >> 15;
                short state_r = ((unsigned short) GetKeyState(VK_RMENU)) >> 15;
                if (state_l)
                {
                    key = (int) KeyCode::LeftAlt;
                }
                else if (state_r)
                {
                    key = (int) KeyCode::RightAlt;
                }
                break;
            }
            case VK_UP:
                key = (int) KeyCode::UpArrow;
                break;
            case VK_DOWN:
                key = (int) KeyCode::DownArrow;
                break;
            case VK_LEFT:
                key = (int) KeyCode::LeftArrow;
                break;
            case VK_RIGHT:
                key = (int) KeyCode::RightArrow;
                break;
            case VK_BACK:
                key = (int) KeyCode::Backspace;
                break;
            case VK_TAB:
                key = (int) KeyCode::Tab;
                break;
            case VK_SPACE:
                key = (int) KeyCode::Space;
                break;
            case VK_ESCAPE:
                key = (int) KeyCode::Escape;
                break;
            case VK_RETURN:
                key = (int) KeyCode::Return;
                break;
            case VK_OEM_3:
                key = (int) KeyCode::BackQuote;
                break;
            case VK_OEM_MINUS:
                key = (int) KeyCode::Minus;
                break;
            case VK_OEM_PLUS:
                key = (int) KeyCode::Equals;
                break;
            case VK_OEM_4:
                key = (int) KeyCode::LeftBracket;
                break;
            case VK_OEM_6:
                key = (int) KeyCode::RightBracket;
                break;
            case VK_OEM_5:
                key = (int) KeyCode::Backslash;
                break;
            case VK_OEM_1:
                key = (int) KeyCode::Semicolon;
                break;
            case VK_OEM_7:
                key = (int) KeyCode::Quote;
                break;
            case VK_OEM_COMMA:
                key = (int) KeyCode::Comma;
                break;
            case VK_OEM_PERIOD:
                key = (int) KeyCode::Period;
                break;
            case VK_OEM_2:
                key = (int) KeyCode::Slash;
                break;
        }
    }

    return key;
}

static void SwitchFullScreen(HWND hWnd)
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
        SetWindowLong(hWnd, GWL_STYLE, WS_OVERLAPPED);
        SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, rect.right, rect.bottom, SWP_SHOWWINDOW);
    }
    else
    {
        full_screen = false;

        SetWindowLong(hWnd, GWL_STYLE, old_style);
        SetWindowPos(hWnd, HWND_NOTOPMOST,
            old_pos.left,
            old_pos.top,
            old_pos.right - old_pos.left,
            old_pos.bottom - old_pos.top,
            SWP_SHOWWINDOW);
    }
}

static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
		case WM_TRAY:
			switch (lParam)
			{
				case WM_RBUTTONDOWN:
				{
					POINT pt;
					GetCursorPos(&pt);
					SetForegroundWindow(hWnd);
					int cmd = TrackPopupMenu(g_tray_menu, TPM_RETURNCMD, pt.x, pt.y, 0, hWnd, nullptr);
					if (cmd == WM_TRAY_OPEN)
					{
						char file_name[260];
						OPENFILENAME ofn;
						ZeroMemory(&ofn, sizeof(ofn));
						ofn.lStructSize = sizeof(ofn);
						ofn.hwndOwner = hWnd;
						ofn.lpstrFile = file_name;
						ofn.lpstrFile[0] = '\0';
						ofn.nMaxFile = sizeof(file_name);
						ofn.lpstrFilter = "Video\0*.mp4\0";
						ofn.nFilterIndex = 1;
						ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

						if (GetOpenFileName(&ofn))
						{
							g_engine->SendMessage(0, file_name);
						}
					}
					else if (cmd == WM_TRAY_EXIT)
					{
						if (g_wallpaper_win != nullptr)
						{
							ShowWindow(g_wallpaper_win, SW_HIDE);
							SetParent(hWnd, nullptr);
						}
						Shell_NotifyIcon(NIM_DELETE, &g_tray_id);
						PostMessage(hWnd, WM_CLOSE, 0, 0);
					}
					break;
				}
				case WM_LBUTTONDOWN:
					MessageBox(hWnd, "WM_LBUTTONDOWN", "", MB_OK);
					break;
			}
			break;

        case WM_CLOSE:
			Engine::Destroy(&g_engine);
            DestroyWindow(hWnd);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_SIZE:
            if (wParam == SIZE_MINIMIZED)
            {
                g_minimized = true;
            }
            else
            {
                if (!g_minimized)
                {
                    int width = lParam & 0xffff;
                    int height = (lParam & 0xffff0000) >> 16;

                    g_window_width = width;
                    g_window_height = height;
                }

                g_minimized = false;
            }
            break;

        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        {
            int key = GetKeyCode((int) wParam);

            if (key >= 0)
            {
                if (!g_key[key])
                {
                    g_key_down[key] = true;
                    g_key[key] = true;
                }
            }
            else
            {
                if (wParam == VK_CAPITAL)
                {
                    short caps_on = ((unsigned short) GetKeyState(VK_CAPITAL)) & 1;
                    g_key[(int) KeyCode::CapsLock] = caps_on == 1;
                }
            }
            break;
        }

        case WM_SYSKEYUP:
        case WM_KEYUP:
        {
            int key = GetKeyCode((int) wParam);

            if (key >= 0)
            {
                g_key_up[key] = true;
                g_key[key] = false;
            }
            else
            {
                switch (wParam)
                {
                    case VK_CONTROL:
                    {
                        if (g_key[(int) KeyCode::LeftControl])
                        {
                            g_key_up[(int) KeyCode::LeftControl] = true;
                            g_key[(int) KeyCode::LeftControl] = false;
                        }
                        if (g_key[(int) KeyCode::RightControl])
                        {
                            g_key_up[(int) KeyCode::RightControl] = true;
                            g_key[(int) KeyCode::RightControl] = false;
                        }
                        break;
                    }
                    case VK_SHIFT:
                    {
                        if (g_key[(int) KeyCode::LeftShift])
                        {
                            g_key_up[(int) KeyCode::LeftShift] = true;
                            g_key[(int) KeyCode::LeftShift] = false;
                        }
                        if (g_key[(int) KeyCode::RightShift])
                        {
                            g_key_up[(int) KeyCode::RightShift] = true;
                            g_key[(int) KeyCode::RightShift] = false;
                        }
                        break;
                    }
                    case VK_MENU:
                    {
                        if (g_key[(int) KeyCode::LeftAlt])
                        {
                            g_key_up[(int) KeyCode::LeftAlt] = true;
                            g_key[(int) KeyCode::LeftAlt] = false;
                        }
                        if (g_key[(int) KeyCode::RightAlt])
                        {
                            g_key_up[(int) KeyCode::RightAlt] = true;
                            g_key[(int) KeyCode::RightAlt] = false;
                        }
                        break;
                    }
                }
            }
            break;
        }

        case WM_CHAR:
            if (wParam > 0 && wParam < 0x10000)
            {
                unsigned short c = (unsigned short) wParam;
                Input::AddInputCharacter(c);
            }
            break;

        case WM_SYSCHAR:
            if (wParam == VK_RETURN)
            {
                // Alt + Enter
                SwitchFullScreen(hWnd);
            }
            break;

        case WM_LBUTTONDOWN:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            if (!g_mouse_down)
            {
                Touch t;
                t.deltaPosition = Vector2(0, 0);
                t.deltaTime = 0;
                t.fingerId = 0;
                t.phase = TouchPhase::Began;
                t.position = Vector2((float) x, (float) g_window_height - y - 1);
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

                g_mouse_down = true;
            }

            g_mouse_button_down[0] = true;
            g_mouse_position.x = (float) x;
            g_mouse_position.y = (float) g_window_height - y - 1;
            g_mouse_button_held[0] = true;

            break;
        }

        case WM_RBUTTONDOWN:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            g_mouse_button_down[1] = true;
            g_mouse_position.x = (float) x;
            g_mouse_position.y = (float) g_window_height - y - 1;
            g_mouse_button_held[1] = true;

            break;
        }

        case WM_MBUTTONDOWN:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            g_mouse_button_down[2] = true;
            g_mouse_position.x = (float) x;
            g_mouse_position.y = (float) g_window_height - y - 1;
            g_mouse_button_held[2] = true;

            break;
        }

        case WM_MOUSEMOVE:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            if (g_mouse_down)
            {
                Touch t;
                t.deltaPosition = Vector2(0, 0);
                t.deltaTime = 0;
                t.fingerId = 0;
                t.phase = TouchPhase::Moved;
                t.position = Vector2((float) x, (float) g_window_height - y - 1);
                t.tapCount = 1;
                t.time = Time::GetRealTimeSinceStartup();

                if (!g_input_touches.Empty())
                {
                    if (g_input_touch_buffer.Empty())
                    {
                        if (g_input_touches[0].phase == TouchPhase::Moved)
                        {
                            g_input_touches[0] = t;
                        }
                        else
                        {
                            g_input_touch_buffer.AddLast(t);
                        }
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
            g_mouse_position.y = (float) g_window_height - y - 1;

            break;
        }

        case WM_LBUTTONUP:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            if (g_mouse_down)
            {
                Touch t;
                t.deltaPosition = Vector2(0, 0);
                t.deltaTime = 0;
                t.fingerId = 0;
                t.phase = TouchPhase::Ended;
                t.position = Vector2((float) x, (float) g_window_height - y - 1);
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

                g_mouse_down = false;
            }

            g_mouse_button_up[0] = true;
            g_mouse_position.x = (float) x;
            g_mouse_position.y = (float) g_window_height - y - 1;
            g_mouse_button_held[0] = false;

            break;
        }

        case WM_RBUTTONUP:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            g_mouse_button_up[1] = true;
            g_mouse_position.x = (float) x;
            g_mouse_position.y = (float) g_window_height - y - 1;
            g_mouse_button_held[1] = false;

            break;
        }

        case WM_MBUTTONUP:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            g_mouse_button_up[2] = true;
            g_mouse_position.x = (float) x;
            g_mouse_position.y = (float) g_window_height - y - 1;
            g_mouse_button_held[2] = false;

            break;
        }

        case WM_MOUSEWHEEL:
        {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            g_mouse_scroll_wheel = delta / 120.0f;
            break;
        }

        default:
            break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

static BOOL CALLBACK EnumWindowsProc(_In_ HWND win, _In_ LPARAM param)
{
	HWND def_view = FindWindowEx(win, nullptr, "SHELLDLL_DefView", nullptr);
	if (def_view != nullptr)
	{
		g_wallpaper_win = FindWindowEx(nullptr, win, "WorkerW", nullptr);
	}
	return TRUE;
}

static void InitTray(HINSTANCE hInstance, HWND hWnd, const char* name, HICON icon)
{
	g_tray_id.cbSize = sizeof(NOTIFYICONDATA);
	g_tray_id.hWnd = hWnd;
	g_tray_id.uID = WM_TRAY;
	g_tray_id.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
	g_tray_id.uCallbackMessage = WM_TRAY;
	g_tray_id.hIcon = icon;
	strcpy(g_tray_id.szTip, name);

	g_tray_menu = CreatePopupMenu();
	AppendMenu(g_tray_menu, MF_STRING, WM_TRAY_OPEN, TEXT("Open Video"));
	AppendMenu(g_tray_menu, MF_STRING, WM_TRAY_EXIT, TEXT("Quit VPaper"));

	Shell_NotifyIcon(NIM_ADD, &g_tray_id);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    String name = "Viry3D";
	int window_width = 1280;
	int window_height = 720;

	bool desktop_window = true;

    WNDCLASSEX win_class;
    ZeroMemory(&win_class, sizeof(win_class));

    win_class.cbSize = sizeof(WNDCLASSEX);
    win_class.style = CS_HREDRAW | CS_VREDRAW;
    win_class.lpfnWndProc = WindowProc;
    win_class.cbClsExtra = 0;
    win_class.cbWndExtra = 0;
    win_class.hInstance = hInstance;
    win_class.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
    win_class.lpszMenuName = nullptr;
    win_class.lpszClassName = name.CString();
    win_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
    win_class.hIcon = (HICON) LoadImage(nullptr, "icon.ico", IMAGE_ICON, SM_CXICON, SM_CYICON, LR_LOADFROMFILE);
    win_class.hIconSm = win_class.hIcon;

    if (!RegisterClassEx(&win_class))
    {
        return 0;
    }

    DWORD style = WS_OVERLAPPEDWINDOW;
    DWORD style_ex = 0;

    RECT wr = { 0, 0, window_width, window_height };
    AdjustWindowRect(&wr, style, FALSE);

	if (desktop_window)
	{
		style = WS_POPUP;
	}

    HWND hwnd = nullptr;

    {
        int x = (GetSystemMetrics(SM_CXSCREEN) - window_width) / 2 + wr.left;
        int y = (GetSystemMetrics(SM_CYSCREEN) - window_height) / 2 + wr.top;
        int w = wr.right - wr.left;
        int h = wr.bottom - wr.top;

        hwnd = CreateWindowEx(
            style_ex,			// window ex style
            name.CString(),		// class name
            name.CString(),		// app name
            style,			    // window style
            x, y,				// x, y
            w, h,               // w, h
            nullptr,		    // handle to parent
            nullptr,            // handle to menu
            hInstance,			// hInstance
            nullptr);           // no extra parameters
    }

    if (!hwnd)
    {
        return 0;
    }

	if (desktop_window)
	{
		DWORD_PTR result = 0;
		HWND win_pm = FindWindow("Progman", nullptr);
		SendMessageTimeout(win_pm, 0x052c, 0, 0, SMTO_NORMAL, 0x3e8, &result);
		EnumWindows(EnumWindowsProc, (LPARAM) nullptr);
		SetParent(hwnd, g_wallpaper_win);
		ShowWindow(g_wallpaper_win, SW_SHOW);
		InitTray(hInstance, hwnd, name.CString(), win_class.hIcon);

        HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONULL);
        if (monitor)
        {
            MONITORINFO monitor_info = { };
            monitor_info.cbSize = sizeof(monitor_info);
            GetMonitorInfo(monitor, &monitor_info);

            window_width = monitor_info.rcMonitor.right - monitor_info.rcMonitor.left;
            window_height = monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top;
            MoveWindow(hwnd, 0, 0, window_width, window_height, true);
        }
	}

    ShowWindow(hwnd, SW_SHOW);

    g_window_width = window_width;
    g_window_height = window_height;

	g_engine = Engine::Create(hwnd, g_window_width, g_window_height);

    bool exit = false;
    MSG msg;

    while (true)
    {
        while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                exit = true;
                break;
            }
            else
            {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
            }
        }

        if (exit)
        {
            break;
        }

		if (g_minimized)
		{
			continue;
		}

		if (g_engine)
		{
			if (g_window_width != g_engine->GetWidth() || g_window_height != g_engine->GetHeight())
			{
				g_engine->OnResize(hwnd, g_window_width, g_window_height);
			}

			g_engine->Execute();

			if (g_engine->HasQuit())
			{
				SendMessageA(hwnd, WM_CLOSE, 0, 0);
			}
		}
    }

#ifndef NDEBUG
	int alloc_size = Memory::GetAllocSize();
	int new_size = Memory::GetNewSize();
	assert(alloc_size == 0);
	assert(new_size == 0);
#endif

    return 0;
}
