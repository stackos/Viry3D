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
#include "memory/ByteBuffer.h"

namespace Viry3D
{
	class File
	{
	public:
		static bool Exist(const String& path);
		static ByteBuffer ReadAllBytes(const String& path);
		static bool WriteAllBytes(const String& path, const ByteBuffer& buffer);
		static String ReadAllText(const String& path);
		static bool WriteAllText(const String& path, const String& text);
        static void Delete(const String& path);
		static void Unzip(const String& path, const String& source, const String& dest, bool directory);
	};
}
