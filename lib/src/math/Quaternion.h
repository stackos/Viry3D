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

#include "Vector3.h"

namespace Viry3D
{
	struct Quaternion
	{
		static Quaternion Identity();
		static Quaternion Inverse(const Quaternion& q);
		static Quaternion AngleAxis(float angle, const Vector3& axis);
		static Quaternion Euler(float x, float y, float z);
		static Quaternion Euler(const Vector3& e) { return Euler(e.x, e.y, e.z); }
		static Quaternion Lerp(const Quaternion& from, const Quaternion& to, float t);
		static Quaternion SLerp(const Quaternion& from, const Quaternion& to, float t);
		static Quaternion FromToRotation(const Vector3& from_direction, const Vector3& to_direction);
		static Quaternion LookRotation(const Vector3& forward, const Vector3& up);

		explicit Quaternion(float x = 0, float y = 0, float z = 0, float w = 1);
		Quaternion operator *(const Quaternion& q) const;
		Quaternion operator *(float v) const;
		Vector3 operator *(const Vector3& p) const;
		bool operator !=(const Quaternion& v) const;
		bool operator ==(const Quaternion& v) const;
		Vector3 ToEulerAngles() const;
		void Normalize();
		float Dot(const Quaternion& v) const;
		String ToString() const;

		float x;
		float y;
		float z;
		float w;
	};
}
