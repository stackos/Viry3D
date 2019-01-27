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
#include "Object.h"

namespace Viry3D
{
    class LuaObject
    {
    public:
        IMPL_INDEX_FUNC();

        static void Set(lua_State* L)
        {
            LuaAPI::SetMetaTable(L, LuaClassType::Object, Index, nullptr, nullptr);

            GetMethods().Add("GetName", GetName);
            GetMethods().Add("SetName", SetName);
        }

    private:
        IMPL_GET_METHODS_FUNC();

        static int GetName(lua_State* L)
        {
            LuaClassPtr* p = (LuaClassPtr*) lua_touserdata(L, 1);

            if (p->ptr_type == LuaPtrType::Raw)
            {
                Object* obj = (Object*) p->ptr;
                lua_pushstring(L, obj->GetName().CString());
                return 1;
            }
            else if (p->ptr_type == LuaPtrType::Shared)
            {
                Ref<Object>* obj = (Ref<Object>*) p->ptr;
                lua_pushstring(L, (*obj)->GetName().CString());
                return 1;
            }

            return 0;
        }

        static int SetName(lua_State* L)
        {
            LuaClassPtr* p = (LuaClassPtr*) lua_touserdata(L, 1);

            const char* name = luaL_checkstring(L, 2);

            if (p->ptr_type == LuaPtrType::Raw)
            {
                Object* obj = (Object*) p->ptr;
                obj->SetName(name);
            }
            else if (p->ptr_type == LuaPtrType::Shared)
            {
                Ref<Object>* obj = (Ref<Object>*) p->ptr;
                (*obj)->SetName(name);
            }

            return 0;
        }
    };
}
