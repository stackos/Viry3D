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

#include "App.h"
#include "graphics/Display.h"
#include "Debug.h"
#include "json/json.h"
#include <emscripten.h>

using namespace Viry3D;

static Display* g_display;
static App* g_app;

extern "C" void EMSCRIPTEN_KEEPALIVE InitEngine(const char* msg)
{
    Log("InitEngine with msg: %s", msg);

    Json::Reader reader;
    Json::Value value;
    if (!reader.parse(msg, value, false))
    {
        Log("InitEngine msg parse error");
    }

    String name = value["name"].asCString();
    int width = value["width"].asInt();
    int height = value["height"].asInt();
    bool glesv3 = value["glesv3"].asBool();

    g_display = new Display(name, nullptr, width, height);
    if (glesv3)
    {
        g_display->EnableGLESv3();
    }

    g_app = new App();
    g_app->SetName(name);
    g_app->Init();

    Log("InitEngine success");
}

extern "C" void EMSCRIPTEN_KEEPALIVE DoneEngine(const char* msg)
{
    Log("DoneEngine with msg: %s", msg);

    delete g_app;
    delete g_display;
}

extern "C" void EMSCRIPTEN_KEEPALIVE UpdateEngine(const char* msg)
{
    g_app->OnFrameBegin();
    g_app->Update();
    g_display->OnDraw();
    g_app->OnFrameEnd();
}

int main()
{
    return 0;
}
