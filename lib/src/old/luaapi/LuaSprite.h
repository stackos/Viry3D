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
#include "ui/Sprite.h"

namespace Viry3D
{
    class LuaSprite
    {
    public:
        IMPL_INDEX_EXTENDS_FUNC(View);

        static void Set(lua_State* L)
        {
            LuaAPI::SetMetaTable(L, LuaClassType::Sprite, Index, New, GC);

            GetMethods().Add("GetTexture", GetTexture);
            GetMethods().Add("SetTexture", SetTexture);
        }

    private:
        IMPL_NEW_FUNC(Sprite);
        IMPL_GC_FUNC(Sprite);
        IMPL_GET_METHODS_FUNC();
        
        static int GetTexture(lua_State* L)
        {
            Sprite* p1 = LuaAPI::GetRawPtr<Sprite>(L, 1);

            Ref<Texture>* ptr = new Ref<Texture>(p1->GetTexture());
            LuaAPI::PushPtr(L, { ptr, LuaClassType::Texture, LuaPtrType::Shared });

            return 1;
        }

        static int SetTexture(lua_State* L)
        {
            Sprite* p1 = LuaAPI::GetRawPtr<Sprite>(L, 1);
            LuaClassPtr* p2 = (LuaClassPtr*) lua_touserdata(L, 2);

            Ref<Texture>* texture = (Ref<Texture>*) p2->ptr;
            p1->SetTexture(*texture);

            return 0;
        }
    };
}
