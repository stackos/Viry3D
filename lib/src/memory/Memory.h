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

#include <stdlib.h>
#include <string.h>

namespace Viry3D
{
	class Memory
	{
	public:
		template<class T>
		inline static T* Alloc(int size) { return (T*) malloc(size); }
        template<class T>
        inline static T* Realloc(T* block, int size) { return (T*) realloc(block, size); }
		inline static void Free(void* block) { free(block); }
		inline static void Zero(void* dest, int size) { memset(dest, 0, size); }
		inline static void Set(void* dest, int value, int size) { memset(dest, value, size); }
		inline static void Copy(void* dest, const void* src, int size) { memcpy(dest, src, size); }
		inline static int Compare(const void* dest, const void* src, int size) { return memcmp(dest, src, size); }

        template<class T>
        inline static void SafeFree(T*& block)
        {
            if (block)
            {
                Memory::Free(block);
                block = nullptr;
            }
        }

        template<class T>
        inline static void SafeDelete(T*& p)
        {
            if (p)
            {
                delete p;
                p = nullptr;
            }
        }
	};
}
