#pragma once

#include "string/String.h"
#include "container/Vector.h"

namespace Viry3D
{
	class Directory
	{
	public:
		static bool Exist(String path);
		static Vector<String> GetDirectorys(String path);
		static Vector<String> GetFiles(String path, bool recursive);
		static void Create(String path);
	};
}