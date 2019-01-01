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

#include "string/String.h"

namespace Viry3D
{
	struct Vector2;

	struct Vector3
	{
		static Vector3 Zero();
		static Vector3 One();
		static Vector3 Normalize(const Vector3& value);
		static float Magnitude(const Vector3& v);
		static float SqrMagnitude(const Vector3& v);
		static Vector3 Max(const Vector3& a, const Vector3& b);
		static Vector3 Min(const Vector3& a, const Vector3& b);
		static Vector3 Lerp(const Vector3& from, const Vector3& to, float t, bool clamp_01 = true);
		static float Angle(const Vector3& from, const Vector3& to);

		explicit Vector3(float x = 0, float y = 0, float z = 0);
		Vector3(const Vector2& v2);
		Vector3 operator -() const;
		Vector3 operator +(const Vector3& v) const;
		Vector3& operator +=(const Vector3& v);
		Vector3 operator -(const Vector3& v) const;
		Vector3 operator *(const Vector3& v) const;
		Vector3 operator *(float v) const;
		Vector3 operator *=(float v);
		bool operator !=(const Vector3& v) const;
		bool operator ==(const Vector3& v) const;
		float Dot(const Vector3& v) const;
		String ToString() const;
		void Normalize();
		float Magnitude() const;
		float SqrMagnitude() const;

		float x;
		float y;
		float z;
	};
}
