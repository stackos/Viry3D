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

#include "LuaAPI.h"
#include "time/Time.h"

namespace Viry3D
{
    class LuaTime
    {
    public:
        static void Set(lua_State* L)
        {
            LuaAPI::SetFunction(L, "Time", "GetFrameCount", GetFrameCount);
            LuaAPI::SetFunction(L, "Time", "GetTime", GetTime);
            LuaAPI::SetFunction(L, "Time", "GetRealTimeSinceStartup", GetRealTimeSinceStartup);
            LuaAPI::SetFunction(L, "Time", "GetDeltaTime", GetDeltaTime);
            LuaAPI::SetFunction(L, "Time", "GetFPS", GetFPS);
        }

    private:
        static int GetFrameCount(lua_State* L)
        {
            lua_pushinteger(L, Time::GetFrameCount());
            return 1;
        }

        static int GetTime(lua_State* L)
        {
            lua_pushnumber(L, Time::GetTime());
            return 1;
        }

        static int GetRealTimeSinceStartup(lua_State* L)
        {
            lua_pushnumber(L, Time::GetRealTimeSinceStartup());
            return 1;
        }

        static int GetDeltaTime(lua_State* L)
        {
            lua_pushnumber(L, Time::GetDeltaTime());
            return 1;
        }

        static int GetFPS(lua_State* L)
        {
            lua_pushinteger(L, Time::GetFPS());
            return 1;
        }
    };
}
