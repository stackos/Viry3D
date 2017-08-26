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
		inline static void Free(void* block) { free(block); }
		inline static void Zero(void* dest, int size) { memset(dest, 0, size); }
		inline static void Set(void* dest, int value, int size) { memset(dest, value, size); }
		inline static void Copy(void* dest, const void* src, int size) { memcpy(dest, src, size); }
		inline static int Compare(const void* dest, const void* src, int size) { return memcmp(dest, src, size); }
	};
}