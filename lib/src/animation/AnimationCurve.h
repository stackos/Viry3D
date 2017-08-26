#pragma once

#include "AnimationWrapMode.h"
#include "container/Vector.h"

namespace Viry3D
{
	struct Keyframe
	{
		Keyframe() { }
		Keyframe(float time, float value):
			in_tangent(0), out_tangent(0), tangent_mode(0), time(time), value(value)
		{
		}
		Keyframe(float time, float value, float in_tangent, float out_tangent):
			in_tangent(in_tangent), out_tangent(out_tangent), tangent_mode(0), time(time), value(value)
		{
		}

		float in_tangent;
		float out_tangent;
		int tangent_mode;
		float time;
		float value;
	};

	struct AnimationCurve
	{
		static AnimationCurve DefaultLinear();
		float Evaluate(float time);

		Vector<Keyframe> keys;
	};
}