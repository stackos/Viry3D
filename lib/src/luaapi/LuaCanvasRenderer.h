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

#include "LuaRenderer.h"
#include "ui/CanvasRenderer.h"

namespace Viry3D
{
    class LuaCanvasRenderer
    {
    public:
        static void Set(lua_State* L)
        {
            LuaAPI::SetMetaTable(L, LuaClassType::CanvasRenderer, Index, New, GC);

            GetMethods().Add("AddView", AddView);
        }

    private:
        IMPL_INDEX_EXTENDS_FUNC(Renderer);
        IMPL_NEW_FUNC(CanvasRenderer);
        IMPL_GC_FUNC(CanvasRenderer);
        IMPL_GET_METHODS_FUNC();

        static int AddView(lua_State* L)
        {
            LuaClassPtr* p1 = (LuaClassPtr*) lua_touserdata(L, 1);
            LuaClassPtr* p2 = (LuaClassPtr*) lua_touserdata(L, 2);

            Ref<View>* view = (Ref<View>*) p2->ptr;

            if (p1->ptr_type == LuaPtrType::Raw)
            {
                CanvasRenderer* obj = (CanvasRenderer*) p1->ptr;
                obj->AddView(*view);
            }
            else if (p1->ptr_type == LuaPtrType::Shared)
            {
                Ref<CanvasRenderer>* obj = (Ref<CanvasRenderer>*) p1->ptr;
                (*obj)->AddView(*view);
            }

            return 0;
        }
    };
}
