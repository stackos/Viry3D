#pragma once

namespace Viry3D
{
	struct AnimationWrapMode
	{
		enum Enum
		{
			Default = 0,
			Once = 1,
			Clamp = 1,
			Loop = 2,
			PingPong = 4,
			ClampForever = 8,
		};
	};
}