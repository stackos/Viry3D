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

#include "LuaView.h"
#include "ui/Label.h"

namespace Viry3D
{
    class LuaLabel
    {
    public:
        static void Set(lua_State* L)
        {
            LuaAPI::SetMetaTable(L, LuaClassType::Label, Index, New, GC);

            GetMethods().Add("SetFont", SetFont);
            GetMethods().Add("SetFontStyle", SetFontStyle);
            GetMethods().Add("SetFontSize", SetFontSize);
            GetMethods().Add("GetText", GetText);
            GetMethods().Add("SetText", SetText);
            GetMethods().Add("SetLineSpace", SetLineSpace);
            GetMethods().Add("SetRich", SetRich);
            GetMethods().Add("SetMono", SetMono);
            GetMethods().Add("SetTextAlignment", SetTextAlignment);
        }

    private:
        IMPL_INDEX_EXTENDS_FUNC(View);
        IMPL_NEW_FUNC(Label);
        IMPL_GC_FUNC(Label);
        IMPL_GET_METHODS_FUNC();

        static int SetFont(lua_State* L)
        {
            Label* p1 = LuaAPI::GetRawPtr<Label>(L, 1);
            LuaClassPtr* p2 = (LuaClassPtr*) lua_touserdata(L, 2);

            Ref<Font>* font = (Ref<Font>*) p2->ptr;
            p1->SetFont(*font);

            return 0;
        }

        static int SetFontStyle(lua_State* L)
        {
            return 0;
        }

        static int SetFontSize(lua_State* L)
        {
            Label* p1 = LuaAPI::GetRawPtr<Label>(L, 1);
            int p2 = (int) luaL_checkinteger(L, 2);

            p1->SetFontSize(p2);

            return 0;
        }

        static int GetText(lua_State* L)
        {
            return 0;
        }

        static int SetText(lua_State* L)
        {
            Label* p1 = LuaAPI::GetRawPtr<Label>(L, 1);
            const char* p2 = luaL_checkstring(L, 2);

            p1->SetText(p2);

            return 0;
        }

        static int SetLineSpace(lua_State* L)
        {
            return 0;
        }

        static int SetRich(lua_State* L)
        {
            return 0;
        }

        static int SetMono(lua_State* L)
        {
            return 0;
        }

        static int SetTextAlignment(lua_State* L)
        {
            Label* p1 = LuaAPI::GetRawPtr<Label>(L, 1);
            int p2 = (int) luaL_checkinteger(L, 2);

            p1->SetTextAlignment(p2);

            return 0;
        }
    };
}
