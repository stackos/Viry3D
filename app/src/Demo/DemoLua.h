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
#include "io/File.h"
#include "luaapi/LuaAPI.h"

namespace Viry3D
{
    class DemoLua : public Demo
    {
    public:
        lua_State* L = nullptr;

        void InitLua()
        {
            L = luaL_newstate();
            luaL_openlibs(L);
            LuaAPI::SetAll(L);

            String lua = File::ReadAllText(Application::Instance()->GetDataPath() + "/lua/DemoLua.lua");

            if (luaL_dostring(L, lua.CString()))
            {
                Debug::LogString(lua_tostring(L, -1), true);
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
                Debug::LogString(lua_tostring(L, -1), true);
                lua_pop(L, 1);
            }
        }

        virtual void Init()
        {
            this->InitLua();
        }

        virtual void Done()
        {
            this->CallGlobalFunction("AppDone");

            lua_close(L);
        }

        virtual void Update()
        {
            this->CallGlobalFunction("AppUpdate");
        }
    };
}
