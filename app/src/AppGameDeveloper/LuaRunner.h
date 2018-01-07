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

#pragma once

#include "Component.h"

namespace Viry3D
{
    enum class LuaTokenType
    {
        Comment,
        Whitespace,
        Function,
        Operator,
        String,
        Number,
        Keyword,
        Identifier
    };

    struct LuaToken
    {
        LuaTokenType type;
        int pos;
    };

	class LuaRunner: public Component
	{
		DECLARE_COM_CLASS(LuaRunner, Component);

	public:
        static Vector<LuaToken> Lex(const String& source);
		void RunSource(const String& source);

	private:
		LuaRunner() { }
	};
}
