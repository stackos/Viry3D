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
#include "ui/View.h"

namespace Viry3D
{
    class LuaView
    {
    public:
        IMPL_INDEX_EXTENDS_FUNC(Object);

        static void Set(lua_State* L)
        {
            LuaAPI::SetMetaTable(L, LuaClassType::View, Index, New, GC);

            GetMethods().Add("GetCanvas", GetCanvas);
            GetMethods().Add("AddSubview", AddSubview);
            GetMethods().Add("RemoveSubview", RemoveSubview);
            GetMethods().Add("ClearSubviews", ClearSubviews);
            GetMethods().Add("GetSubviewCount", GetSubviewCount);
            GetMethods().Add("GetParentView", GetParentView);
            GetMethods().Add("GetColor", GetColor);
            GetMethods().Add("SetColor", SetColor);
            GetMethods().Add("GetAlignment", GetAlignment);
            GetMethods().Add("SetAlignment", SetAlignment);
            GetMethods().Add("GetPivot", GetPivot);
            GetMethods().Add("SetPivot", SetPivot);
            GetMethods().Add("GetSize", GetSize);
            GetMethods().Add("SetSize", SetSize);
            GetMethods().Add("GetOffset", GetOffset);
            GetMethods().Add("SetOffset", SetOffset);
            GetMethods().Add("GetLocalRotation", GetLocalRotation);
            GetMethods().Add("SetLocalRotation", SetLocalRotation);
            GetMethods().Add("GetLocalScale", GetLocalScale);
            GetMethods().Add("SetLocalScale", SetLocalScale);
        }

    private:
        IMPL_NEW_FUNC(View);
        IMPL_GC_FUNC(View);
        IMPL_GET_METHODS_FUNC();

        static int GetCanvas(lua_State* L)
        {
            return 0;
        }

        static int AddSubview(lua_State* L)
        {
            return 0;
        }

        static int RemoveSubview(lua_State* L)
        {
            return 0;
        }

        static int ClearSubviews(lua_State* L)
        {
            return 0;
        }

        static int GetSubviewCount(lua_State* L)
        {
            return 0;
        }

        static int GetSubview(lua_State* L)
        {
            return 0;
        }

        static int GetParentView(lua_State* L)
        {
            return 0;
        }

        static int GetColor(lua_State* L)
        {
            return 0;
        }

        static int SetColor(lua_State* L)
        {
            return 0;
        }

        static int GetAlignment(lua_State* L)
        {
            return 0;
        }

        static int SetAlignment(lua_State* L)
        {
            return 0;
        }

        static int GetPivot(lua_State* L)
        {
            return 0;
        }

        static int SetPivot(lua_State* L)
        {
            return 0;
        }

        static int GetSize(lua_State* L)
        {
            return 0;
        }

        static int SetSize(lua_State* L)
        {
            return 0;
        }

        static int GetOffset(lua_State* L)
        {
            return 0;
        }

        static int SetOffset(lua_State* L)
        {
            return 0;
        }

        static int GetLocalRotation(lua_State* L)
        {
            return 0;
        }

        static int SetLocalRotation(lua_State* L)
        {
            return 0;
        }

        static int GetLocalScale(lua_State* L)
        {
            return 0;
        }

        static int SetLocalScale(lua_State* L)
        {
            return 0;
        }
    };
}
