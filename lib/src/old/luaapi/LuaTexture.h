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
#include "graphics/Texture.h"

namespace Viry3D
{
    class LuaTexture
    {
    public:
        static void Set(lua_State* L)
        {
            LuaAPI::SetFunction(L, "Texture", "LoadTexture2DFromFile", LoadTexture2DFromFile);

            LuaAPI::SetMetaTable(L, LuaClassType::Texture, Index, nullptr, GC);

            // FilterMode
            LuaAPI::SetEnum(L, "FilterMode", "None", (int) FilterMode::None);
            LuaAPI::SetEnum(L, "FilterMode", "Nearest", (int) FilterMode::Nearest);
            LuaAPI::SetEnum(L, "FilterMode", "Linear", (int) FilterMode::Linear);
            LuaAPI::SetEnum(L, "FilterMode", "Trilinear", (int) FilterMode::Trilinear);

            // SamplerAddressMode
            LuaAPI::SetEnum(L, "SamplerAddressMode", "None", (int) SamplerAddressMode::None);
            LuaAPI::SetEnum(L, "SamplerAddressMode", "Repeat", (int) SamplerAddressMode::Repeat);
            LuaAPI::SetEnum(L, "SamplerAddressMode", "ClampToEdge", (int) SamplerAddressMode::ClampToEdge);
            LuaAPI::SetEnum(L, "SamplerAddressMode", "Mirror", (int) SamplerAddressMode::Mirror);
            LuaAPI::SetEnum(L, "SamplerAddressMode", "MirrorOnce", (int) SamplerAddressMode::MirrorOnce);
        }

    private:
        IMPL_INDEX_EXTENDS_FUNC(Object);
        IMPL_GC_FUNC(Texture);
        IMPL_GET_METHODS_FUNC();

        static int LoadTexture2DFromFile(lua_State* L)
        {
            const char* p1 = luaL_checkstring(L, 1);
            int p2 = (int) luaL_checkinteger(L, 2);
            int p3 = (int) luaL_checkinteger(L, 3);
            int n = Top();
            bool gen_mipmap = false;
            bool is_storage = false;
            if (n >= 4)
            {
                gen_mipmap = lua_toboolean(L, 4);
            }
            if (n >= 5)
            {
                is_storage = lua_toboolean(L, 5);
            }

            Ref<Texture> texture = Texture::LoadTexture2DFromFile(
                p1,
                (FilterMode) p2,
                (SamplerAddressMode) p3,
                gen_mipmap,
                is_storage);

            Ref<Texture>* ptr = new Ref<Texture>(texture);
            LuaAPI::PushPtr(L, { ptr, LuaClassType::Texture, LuaPtrType::Shared });

            return 1;
        }
    };
}
