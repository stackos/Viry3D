#pragma once

namespace Viry3D
{
	struct RenderQueue
	{
		enum Enum
		{
			Background = 1000,
			Geometry = 2000,
			AlphaTest = 2450,
			Transparent = 3000,
			Overlay = 4000,
		};
	};
}