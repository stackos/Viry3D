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

#pragma once

#include "Demo.h"
#include "Application.h"
#include "graphics/Display.h"
#include "graphics/Camera.h"
#include "time/Time.h"
#include "ui/CanvasRenderer.h"
#include "ui/Label.h"
#include "ui/Font.h"
#include "lua/lua.hpp"

namespace Viry3D
{
    class DemoLua : public Demo
    {
    public:
        lua_State* L = nullptr;
        Camera* m_ui_camera = nullptr;
        Label* m_label = nullptr;

        void InitLua()
        {
            L = luaL_newstate();
            luaL_openlibs(L);

            String lua = R"(
function AppInit()
end

function AppDone()
end

function AppUpdate()
end
)";

            if (luaL_dostring(L, lua.CString()))
            {
                Log("%s\n", lua_tostring(L, -1));
                lua_pop(L, 1);
                return;
            }

            this->CallGlobalFunction("AppInit");
        }

        void CallGlobalFunction(const char* name)
        {
            lua_getglobal(L, name);
            if (lua_pcall(L, 0, 0, 0))
            {
                Log("%s\n", lua_tostring(L, -1));
                lua_pop(L, 1);
            }
        }

        void InitUI()
        {
            m_ui_camera = Display::Instance()->CreateCamera();

            auto canvas = RefMake<CanvasRenderer>();
            m_ui_camera->AddRenderer(canvas);

            auto label = RefMake<Label>();
            canvas->AddView(label);

            label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetPivot(Vector2(0, 0));
            label->SetSize(Vector2i(100, 30));
            label->SetOffset(Vector2i(40, 40));
            label->SetFont(Font::GetFont(FontType::Consola));
            label->SetFontSize(28);
            label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);

            m_label = label.get();
        }

        virtual void Init()
        {
            this->InitUI();
            this->InitLua();
        }

        virtual void Done()
        {
            Display::Instance()->DestroyCamera(m_ui_camera);

            this->CallGlobalFunction("AppDone");

            lua_close(L);
        }

        virtual void Update()
        {
            this->CallGlobalFunction("AppUpdate");

            if (m_label)
            {
                m_label->SetText(String::Format("FPS:%d", Time::GetFPS()));
            }
        }
    };
}
