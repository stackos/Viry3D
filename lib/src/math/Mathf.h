/*
* Viry3D
* Copyright 2014-2019 by Stack - stackos@qq.com
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

#include <math.h>
#include "Vector3.h"
#include "Ray.h"
#include "Bounds.h"
#include "container/Vector.h"

namespace Viry3D
{
	class Mathf
	{
	public:
		static const float Epsilon;
		static const float PI;
		static const float Deg2Rad;
		static const float Rad2Deg;
		static const float MaxFloatValue;
		static const float MinFloatValue;
        static const float ToLinearSpace;

		template<class T>
		static T Max(T a, T b) { return a > b ? a : b; }
		template<class T>
		inline static T Max(const Vector<T>& a);
		template<class T>
		static T Min(T a, T b) { return a < b ? a : b; }
		template<class T>
		inline static T Min(const Vector<T>& a);
		static float Clamp01(float value) { return Min<float>(Max<float>(value, 0), 1); }
		template<class T>
		static T Clamp(T value, T min, T max) { return Min(Max(value, min), max); }
		static float Lerp(float from, float to, float t, bool clamp_01 = true);
		static bool FloatEqual(float a, float b) { return fabs(a - b) < Epsilon; }
		static float Round(float f);//ËÄÉáÎåÈë
		static int RoundToInt(float f);
		static float Sign(float f) { return f < 0 ? -1.0f : 1.0f; }
		template<class T>
		static void Swap(T& a, T& b) { T temp = a; a = b; b = temp; }
		static float RandomRange(float min, float max);
		static int RandomRange(int min, int max);
		static float Log2(float x) { return logf(x) / logf(2); }
		static int Abs(int v) { return (int) fabsf((float) v); }
        static bool RayPlaneIntersection(const Ray& ray, const Vector3& plane_normal, const Vector3& plane_point, float& ray_length);
        static bool RayBoundsIntersection(const Ray& ray, const Bounds& box, float& ray_length);
	};

	template<class T>
	T Mathf::Max(const Vector<T>& a)
	{
		T max = a[0];

		for (int i = 1; i < a.Size(); ++i)
		{
			if (a[i] > max)
			{
				max = a[i];
			}
		}

		return max;
	}

	template<class T>
	T Mathf::Min(const Vector<T>& a)
	{
		T min = a[0];

		for (int i = 1; i < a.Size(); ++i)
		{
			if (a[i] < min)
			{
				min = a[i];
			}
		}

		return min;
	}
}
