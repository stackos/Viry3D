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

#include "LuaAPI.h"
#include "LuaDisplay.h"
#include "LuaObject.h"
#include "LuaNode.h"
#include "LuaCamera.h"
#include "LuaRenderer.h"
#include "LuaCanvasRenderer.h"
#include "LuaView.h"
#include "LuaLabel.h"

namespace Viry3D
{
    static int PrintLog(lua_State* L)
    {
        int n = Top();
        lua_getglobal(L, "tostring");
        for (int i = 1; i <= n; ++i)
        {
            lua_pushvalue(L, -1);
            lua_pushvalue(L, i);
            lua_call(L, 1, 1);
            const char* s = lua_tostring(L, -1);
            if (s == nullptr)
            {
                return luaL_error(L, "'tostring' must return a string to 'print'");
            }
            if (i > 1)
            {
                Debug::LogString("\t", false);
            }
            Debug::LogString(s, false);
            Pop(); // pop string
        }
        Debug::LogString("", true);
        Pop(); // pop tostring function
        return 0;
    }

    const char* LuaAPI::GetLuaClassName(LuaClassType type)
    {
        static const char* s_class_names[(int) LuaClassType::Count] = {
            "Application",
            "Display",
            "Object",
            "Node",
            "Camera",
            "Renderer",
            "CanvasRenderer",
            "View",
            "Label",
            "Sprite",
            "Font",
            "Texture",
            "Color",
            "Vector2",
            "Vector2i",
            "Quaternion",
            "Time",
        };
        return s_class_names[(int) type];
    }

    void LuaAPI::SetAll(lua_State* L)
    {
        SetFunction(L, "_G", "print", PrintLog);
        
        // LuaApplication::Set(L);
        LuaDisplay::Set(L);
        LuaObject::Set(L);
        LuaNode::Set(L);
        LuaCamera::Set(L);
        LuaRenderer::Set(L);
        LuaCanvasRenderer::Set(L);
        LuaView::Set(L);
        LuaLabel::Set(L);
        // LuaSprite::Set(L);
        // LuaFont::Set(L);
        // LuaTexture::Set(L);
        // LuaColor::Set(L);
        // LuaVector2::Set(L);
        // LuaVector2i::Set(L);
        // LuaQuaternion::Set(L);
        // LuaTime::Set(L);
    }

    void LuaAPI::SetFunction(lua_State* L, const char* table, const char* name, lua_CFunction func)
    {
        lua_getglobal(L, table);
        if (!lua_istable(L, -1))
        {
            Pop();
            lua_newtable(L);
            lua_setglobal(L, table);
            lua_getglobal(L, table);
        }
        lua_pushcfunction(L, func);
        lua_setfield(L, -2, name);
        Pop();
    }

    void LuaAPI::SetMetaTable(lua_State* L, LuaClassType type, lua_CFunction index, lua_CFunction alloc, lua_CFunction gc)
    {
        const char* name = GetLuaClassName(type);

        luaL_newmetatable(L, name);
        if (index)
        {
            lua_pushcfunction(L, index);
            lua_setfield(L, -2, "__index");
        }
        if (alloc)
        {
            LuaAPI::SetFunction(L, name, "New", alloc);

            if (gc)
            {
                lua_pushcfunction(L, gc);
                lua_setfield(L, -2, "__gc");
            }
            else
            {
                assert(!"has alloc function but no gc");
            }
        }
        Pop();
    }

    void LuaAPI::PushPtr(lua_State* L, const LuaClassPtr& value)
    {
        LuaClassPtr* userdata = (LuaClassPtr*) lua_newuserdata(L, sizeof(LuaClassPtr));
        *userdata = value;
        luaL_setmetatable(L, LuaAPI::GetLuaClassName(value.class_type));
    }

    void* LuaAPI::GetPtr(lua_State* L, int index, LuaClassType class_type)
    {
        LuaClassPtr* userdata = (LuaClassPtr*) luaL_checkudata(L, index, LuaAPI::GetLuaClassName(class_type));
        if (userdata)
        {
            return userdata->ptr;
        }
        return nullptr;
    }
}
