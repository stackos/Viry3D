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

#include "Mathf.h"
#include <stdlib.h>

namespace Viry3D
{
	const float Mathf::Epsilon = 0.00001f;
	const float Mathf::PI = 3.1415926f;
	const float Mathf::Deg2Rad = 0.0174533f;
	const float Mathf::Rad2Deg = 57.2958f;
	const float Mathf::MaxFloatValue = 3.402823466e+38F;
	const float Mathf::MinFloatValue = -Mathf::MaxFloatValue;

	float Mathf::Lerp(float from, float to, float t, bool clamp_01)
	{
		if (clamp_01)
		{
			t = Mathf::Clamp01(t);
		}

		return from + (to - from) * t;
	}

	float Mathf::Round(float f)
	{
		float up = ceil(f);
		float down = floor(f);

		if (f - down < 0.5f)
		{
			return down;
		}
		else if (up - f < 0.5f)
		{
			return up;
		}
		else
		{
			return ((((int) up) % 2) == 0 ? up : down);
		}
	}

	int Mathf::RoundToInt(float f)
	{
		return (int) Round(f);
	}

	float Mathf::RandomRange(float min, float max)
	{
		long long rand_max = (long long) RAND_MAX + 1;
		double rand_01 = rand() / (double) rand_max;

		return (float) (min + rand_01 * (max - min));
	}

	int Mathf::RandomRange(int min, int max)
	{
		return (int) (min + RandomRange(0.0f, 1.0f) * (max - min));
	}
}
