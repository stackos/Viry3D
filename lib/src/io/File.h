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
		static void WriteAllBytes(const String& path, const ByteBuffer& buffer);
		static String ReadAllText(const String& path);
		static void WriteAllText(const String& path, const String& text);
		static void Unzip(const String& path, const String& source, const String& dest, bool directory);
	};
}