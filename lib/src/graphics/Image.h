#pragma once

#include "string/String.h"

namespace Viry3D
{
	class Texture2D;

	class Image
	{
	public:
		static ByteBuffer LoadJPEG(const ByteBuffer& jpeg, int& width, int& height, int& bpp);
		static ByteBuffer LoadPNG(const ByteBuffer& png, int& width, int& height, int& bpp);
		static void EncodeToPNG(Texture2D *tex, int bpp, String file);
	};
}