#pragma once

namespace Viry3D
{
	struct CameraClearFlags
	{
		enum Enum
		{
			Invalidate = 1,
			Color = 2,
			Depth = 3,
			Nothing = 4,
		};
	};
}