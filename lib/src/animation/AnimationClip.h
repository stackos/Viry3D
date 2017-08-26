#pragma once

#include "Object.h"
#include "AnimationCurve.h"
#include "AnimationWrapMode.h"

namespace Viry3D
{
	struct CurveProperty
	{
		enum Enum
		{
			LocalPosX,
			LocalPosY,
			LocalPosZ,
			LocalRotX,
			LocalRotY,
			LocalRotZ,
			LocalRotW,
			LocalScaX,
			LocalScaY,
			LocalScaZ,

			Count
		};
	};

	struct CurveBinding
	{
		String path;
		Vector<AnimationCurve> curves;
	};

	class AnimationClip : public Object
	{
	public:
		float frame_rate;
		float length;
		AnimationWrapMode::Enum wrap_mode;
		Map<String, CurveBinding> curves;
	};
}