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

#include "graphics/Display.h"
#include "App.h"
#include "Input.h"
#include "time/Time.h"
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
        case WM_CLOSE:
            DestroyWindow(hWnd);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_SIZE:
            if (wParam == SIZE_MINIMIZED)
            {
                g_minimized = true;

                Display::Instance()->OnPause();
            }
            else
            {
                if (g_minimized)
                {
                    Display::Instance()->OnResume();
                }
                else
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
                t.position = Vector2((float) x, (float) Display::Instance()->GetHeight() - y - 1);
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
            g_mouse_position.y = (float) Display::Instance()->GetHeight() - y - 1;
            g_mouse_button_held[0] = true;

            break;
        }

        case WM_RBUTTONDOWN:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            g_mouse_button_down[1] = true;
            g_mouse_position.x = (float) x;
            g_mouse_position.y = (float) Display::Instance()->GetHeight() - y - 1;
            g_mouse_button_held[1] = true;

            break;
        }

        case WM_MBUTTONDOWN:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            g_mouse_button_down[2] = true;
            g_mouse_position.x = (float) x;
            g_mouse_position.y = (float) Display::Instance()->GetHeight() - y - 1;
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
                t.position = Vector2((float) x, (float) Display::Instance()->GetHeight() - y - 1);
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
            g_mouse_position.y = (float) Display::Instance()->GetHeight() - y - 1;

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
                t.position = Vector2((float) x, (float) Display::Instance()->GetHeight() - y - 1);
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
            g_mouse_position.y = (float) Display::Instance()->GetHeight() - y - 1;
            g_mouse_button_held[0] = false;

            break;
        }

        case WM_RBUTTONUP:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            g_mouse_button_up[1] = true;
            g_mouse_position.x = (float) x;
            g_mouse_position.y = (float) Display::Instance()->GetHeight() - y - 1;
            g_mouse_button_held[1] = false;

            break;
        }

        case WM_MBUTTONUP:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            g_mouse_button_up[2] = true;
            g_mouse_position.x = (float) x;
            g_mouse_position.y = (float) Display::Instance()->GetHeight() - y - 1;
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    String name = "viry3d-vk-demo";
    int window_width = 1280;
    int window_height = 720;

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

    int x = (GetSystemMetrics(SM_CXSCREEN) - window_width) / 2 + wr.left;
    int y = (GetSystemMetrics(SM_CYSCREEN) - window_height) / 2 + wr.top;

    HWND hwnd = CreateWindowEx(
        style_ex,			// window ex style
        name.CString(),		// class name
        name.CString(),		// app name
        style,			    // window style
        x, y,				// x, y
        wr.right - wr.left, // width
        wr.bottom - wr.top, // height
        nullptr,		    // handle to parent
        nullptr,            // handle to menu
        hInstance,			// hInstance
        nullptr);           // no extra parameters
    if (!hwnd)
    {
        return 0;
    }

    ShowWindow(hwnd, SW_SHOW);

    Display* display = new Display(name, hwnd, window_width, window_height);

    App* app = new App();
    app->SetName(name);
    app->Init();

    g_window_width = window_width;
    g_window_height = window_height;

    bool exit = false;
    MSG msg;

    while (true)
    {
        while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (WM_QUIT == msg.message)
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

        if (g_window_width != display->GetWidth() || g_window_height != display->GetHeight())
        {
            display->OnResize(g_window_width, g_window_height);
        }

        app->OnFrameBegin();

        app->Update();

        if (app->HasQuit())
        {
            SendMessage(hwnd, WM_CLOSE, 0, 0);
        }
        else
        {
            display->OnDraw();
        }

        app->OnFrameEnd();
    }

    delete app;
    delete display;

    return 0;
}
