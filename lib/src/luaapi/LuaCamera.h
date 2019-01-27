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

#include "LuaNode.h"
#include "graphics/Camera.h"
#include "ui/CanvasRenderer.h"

namespace Viry3D
{
    class LuaCamera
    {
    public:
        static void Set(lua_State* L)
        {
            LuaAPI::SetMetaTable(L, LuaClassType::Camera, Index, nullptr, nullptr);

            GetMethods().Add("AddRenderer", AddRenderer);
        }

    private:
        IMPL_INDEX_EXTENDS_FUNC(Node);
        IMPL_GET_METHODS_FUNC();

        static int AddRenderer(lua_State* L)
        {
            Camera* camera = (Camera*) LuaAPI::GetPtr(L, 1, LuaClassType::Camera);
            LuaClassPtr* p2 = (LuaClassPtr*) lua_touserdata(L, 2);

            Ref<Renderer>* renderer = (Ref<Renderer>*) p2->ptr;

            camera->AddRenderer(*renderer);

            return 0;
        }
    };
}
