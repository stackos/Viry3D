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
#include "Debug.h"
#include "Input.h"
#include "time/Time.h"
#include "json/json.h"
#include <emscripten.h>

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
static Engine* g_engine;

enum class EventType
{
    MouseDown = 0,
    MouseMove,
    MouseUp,
    MouseWheel,
    KeyDown,
    KeyUp,
    InputCharacter,
};

extern "C" void EMSCRIPTEN_KEEPALIVE InitEngine(const char* msg)
{
    Log("InitEngine with msg: %s", msg);

    Json::Reader reader;
    Json::Value value;
    if (!reader.parse(msg, value, false))
    {
        Log("InitEngine msg parse error");
        return;
    }

    String name = value["name"].asCString();
    int width = value["width"].asInt();
    int height = value["height"].asInt();
    bool glesv3 = value["glesv3"].asBool();
    int platform = value["platform"].asInt();
	
	g_engine = Engine::Create(nullptr, width, height);

    Log("InitEngine success");
}

extern "C" void EMSCRIPTEN_KEEPALIVE DoneEngine(const char* msg)
{
    Log("DoneEngine with msg: %s", msg);

	Engine::Destroy(&g_engine);
}

extern "C" void EMSCRIPTEN_KEEPALIVE UpdateEngine(const char* msg)
{
    Json::Reader reader;
    Json::Value value;
    if (!reader.parse(msg, value, false))
    {
        Log("UpdateEngine msg parse error");
        return;
    }

    const Json::Value& events = value["events"];
    for (int i = 0; i < events.size(); ++i)
    {
        const Json::Value& e = events[i];
        EventType type = (EventType) e["type"].asInt();
        
        switch (type)
        {
            case EventType::MouseDown:
            {
                int x = e["x"].asInt();
                int y = e["y"].asInt();

                if (!g_mouse_down)
                {
                    g_mouse_down = true;

                    Touch t;
                    t.deltaPosition = Vector2(0, 0);
                    t.deltaTime = 0;
                    t.fingerId = 0;
                    t.phase = TouchPhase::Began;
                    t.position = Vector2((float) x, (float) g_engine->GetHeight() - y - 1);
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
                }

                g_mouse_button_down[0] = true;
                g_mouse_position.x = (float) x;
                g_mouse_position.y = (float) g_engine->GetHeight() - y - 1;
                g_mouse_button_held[0] = true;

                break;
            }
                
            case EventType::MouseMove:
            {
                int x = e["x"].asInt();
                int y = e["y"].asInt();

                if (g_mouse_down)
                {
                    Touch t;
                    t.deltaPosition = Vector2(0, 0);
                    t.deltaTime = 0;
                    t.fingerId = 0;
                    t.phase = TouchPhase::Moved;
                    t.position = Vector2((float) x, (float) g_engine->GetHeight() - y - 1);
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
                g_mouse_position.y = (float) g_engine->GetHeight() - y - 1;

                break;
            }

            case EventType::MouseUp:
            {
                int x = e["x"].asInt();
                int y = e["y"].asInt();

                if (g_mouse_down)
                {
                    g_mouse_down = false;

                    Touch t;
                    t.deltaPosition = Vector2(0, 0);
                    t.deltaTime = 0;
                    t.fingerId = 0;
                    t.phase = TouchPhase::Ended;
                    t.position = Vector2((float) x, (float) g_engine->GetHeight() - y - 1);
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
                }

                g_mouse_button_up[0] = true;
                g_mouse_position.x = (float) x;
                g_mouse_position.y = (float) g_engine->GetHeight() - y - 1;
                g_mouse_button_held[0] = false;

                break;
            }

            case EventType::MouseWheel:
            {
                int delta = e["delta"].asInt();

                g_mouse_scroll_wheel = -delta / 3.0f;

                break;
            }

            case EventType::KeyDown:
            {
                int key = e["key"].asInt();

                if (key >= 0)
                {
                    if (!g_key[key])
                    {
                        g_key_down[key] = true;
                        g_key[key] = true;
                    }
                }

                break;
            }

            case EventType::KeyUp:
            {
                int key = e["key"].asInt();

                if (key >= 0)
                {
                    g_key_up[key] = true;
                    g_key[key] = false;
                }

                break;
            }

            case EventType::InputCharacter:
            {
                int key = e["key"].asInt();

                Input::AddInputCharacter((unsigned short) key);

                break;
            }
        }
    }

	g_engine->Execute();
}

int main()
{
    return 0;
}
