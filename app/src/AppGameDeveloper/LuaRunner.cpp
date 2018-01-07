/*
* Viry3D
* Copyright 2014-2018 by Stack - stackos@qq.com
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

#include "LuaRunner.h"
#include "Debug.h"
#include "Application.h"
#include "lua/lua.hpp"

using namespace Viry3D;

extern "C"
{
    extern int luaopen_lpeg(lua_State *L);

	static int lua_print_log(lua_State* L)
	{
		int n = lua_gettop(L);  /* number of arguments */
		int i;
		lua_getglobal(L, "tostring");
		for (i = 1; i <= n; i++)
		{
			const char *s;
			size_t l;
			lua_pushvalue(L, -1);  /* function to be called */
			lua_pushvalue(L, i);   /* value to print */
			lua_call(L, 1, 1);
			s = lua_tolstring(L, -1, &l);  /* get result */
			if (s == NULL)
				return luaL_error(L, "'tostring' must return a string to 'print'");
			if (i > 1)
			{
				//lua_writestring("\t", 1);
				Debug::LogString("\t", false);
			}
			//lua_writestring(s, l);
			Debug::LogString(s, false);
			lua_pop(L, 1);  /* pop result */
		}
		//lua_writeline();
		Debug::LogString("\n", false);

		// print call stack
		/*luaL_traceback(L, L, NULL, 1);
		Debug::LogString(lua_tostring(L, -1), false);
		Debug::LogString("\n", false);
        lua_pop(L, 1);*/

        lua_settop(L, n);

		return 0;
	}

	static const luaL_Reg print_funcs[] = {
		{ "print", lua_print_log },
		{ NULL, NULL }
	};

	static void register_print_func(lua_State* L)
	{
		lua_pushglobaltable(L);
		luaL_setfuncs(L, print_funcs, 0);
		lua_pop(L, 1);
	}

    static void register_lpeg_model(lua_State* L)
    {
        luaL_requiref(L, "lpeg", luaopen_lpeg, 1);
        lua_pop(L, 1);
    }

    static void set_package_path(lua_State* L)
    {
        lua_getglobal(L, "package");

        lua_getfield(L, -1, "path");
        String path = lua_tostring(L, -1);
        lua_pop(L, 1);

        path += ";" + Application::DataPath() + "/lua/?.lua;"
            + Application::DataPath() + "/lua/lexers/?.lua";
        lua_pushstring(L, path.CString());
        lua_setfield(L, -2, "path");

        lua_pop(L, 1);
    }
}

namespace Viry3D
{
	DEFINE_COM_CLASS(LuaRunner);

	void LuaRunner::DeepCopy(const Ref<Object>& source)
	{
		Component::DeepCopy(source);
	}

	void LuaRunner::RunSource(const String& source)
	{
		lua_State* L = luaL_newstate();

		luaL_openlibs(L);
		register_print_func(L);
        register_lpeg_model(L);
        set_package_path(L);

		auto error = luaL_dostring(L, source.CString());
        if (error)
        {
            String error_str = lua_tostring(L, -1);
            lua_pop(L, 1);

            Log("LuaRunner::RunSource error:%s", error_str.CString());
        }

		lua_close(L);
	}
}
