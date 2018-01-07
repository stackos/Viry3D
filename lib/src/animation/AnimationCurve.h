/*
* Viry3D
* Copyright 2014-2018 by Stack - stackos@qq.com
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

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
