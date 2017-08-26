#include "Screen.h"
#include "Graphics.h"

namespace Viry3D
{
	int Screen::GetWidth()
	{
		return Graphics::GetDisplay()->GetWidth();
	}

	int Screen::GetHeight()
	{
		return Graphics::GetDisplay()->GetHeight();
	}
}