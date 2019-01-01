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

#include "string/String.h"
#include <assert.h>

namespace Viry3D
{
	class Debug
	{
	public:
		static void LogString(const String& str, bool end_line);
	};

#define Log(...) Viry3D::Debug::LogString(Viry3D::String::Format(__VA_ARGS__) + Viry3D::String::Format("\n<=[%s:%d]", __FILE__, __LINE__), true)

#define LogGLError()						\
    {										\
        int err = glGetError();				\
		if(err != 0)						\
		{									\
            Log("glGetError: %d", err);		\
        }									\
    }
}
