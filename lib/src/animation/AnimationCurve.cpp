/*
* Viry3D
* Copyright 2014-2017 by Stack - stackos@qq.com
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

#include "AnimationCurve.h"
#include <math.h>
#include "math/Mathf.h"

namespace Viry3D
{
	AnimationCurve AnimationCurve::DefaultLinear()
	{
		AnimationCurve c;
		c.keys.Add(Keyframe(0, 0, 1, 1));
		c.keys.Add(Keyframe(1, 1, 1, 1));

		return c;
	}

	static float evaluate(float time, const Keyframe& k0, const Keyframe& k1)
	{
		float dt = k1.time - k0.time;
		if (fabs(dt) < Mathf::Epsilon)
		{
			return k0.value;
		}

		float t = (time - k0.time) / dt;
		float t2 = t * t;
		float t3 = t2 * t;
		float _t = 1 - t;
		float _t2 = _t * _t;
		float _t3 = _t2 * _t;

		float c = 1 / 3.0f;
		float c0 = dt * c * k0.out_tangent + k0.value;
		float c1 = -dt * c * k1.in_tangent + k1.value;
		float value = k0.value * _t3 + 3 * c0 * t * _t2 + 3 * c1 * t2 * _t + k1.value * t3;

		return value;
	}

	float AnimationCurve::Evaluate(float time)
	{
		if (keys.Empty())
		{
			return 0;
		}

		const auto& back = keys[keys.Size() - 1];
		if (time >= back.time)
		{
			return back.value;
		}

		for (int i = 0; i < keys.Size(); i++)
		{
			const auto& frame = keys[i];

			if (time < frame.time)
			{
				if (i == 0)
				{
					return frame.value;
				}
				else
				{
					return evaluate(time, keys[i - 1], frame);
				}
			}
		}

		return 0;
	}
}
