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

#include "LuaObject.h"
#include "ui/Font.h"

namespace Viry3D
{
    class LuaFont
    {
    public:
        static void Set(lua_State* L)
        {
            LuaAPI::SetFunction(L, "Font", "GetFont", GetFont);
            LuaAPI::SetFunction(L, "Font", "LoadFromFile", LoadFromFile);

            LuaAPI::SetMetaTable(L, LuaClassType::Font, nullptr, nullptr, GC);

            // FontType
            LuaAPI::SetEnum(L, "FontType", "Arial", (int) FontType::Arial);
            LuaAPI::SetEnum(L, "FontType", "Consola", (int) FontType::Consola);
            LuaAPI::SetEnum(L, "FontType", "PingFangSC", (int) FontType::PingFangSC);
            LuaAPI::SetEnum(L, "FontType", "SimSun", (int) FontType::SimSun);
        }

    private:
        IMPL_GC_FUNC(Font);

        static int GetFont(lua_State* L)
        {
            int p1 = (int) luaL_checkinteger(L, 1);
            Ref<Font>* ptr = new Ref<Font>(Font::GetFont((FontType) p1));
            LuaAPI::PushPtr(L, { ptr, LuaClassType::Font, LuaPtrType::Shared });
            return 1;
        }

        static int LoadFromFile(lua_State* L)
        {
            const char* p1 = luaL_checkstring(L, 1);
            Ref<Font>* ptr = new Ref<Font>(Font::LoadFromFile(p1));
            LuaAPI::PushPtr(L, { ptr, LuaClassType::Font, LuaPtrType::Shared });
            return 1;
        }
    };
}
