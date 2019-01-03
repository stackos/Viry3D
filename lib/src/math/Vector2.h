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
	struct Vector3;

	struct Vector2
	{
		explicit Vector2(float x = 0, float y = 0): x(x), y(y) { }
		Vector2(const Vector3& v3);
		Vector2 operator +(const Vector2& value) const;
        Vector2& operator +=(const Vector2& value);
		Vector2 operator -(const Vector2& value) const;
        Vector2& operator -=(const Vector2& value);
        float operator *(const Vector2& v) const { return x * v.y - y * v.x; }
        Vector2 operator *(float value) const;
        Vector2& operator *=(float value);
		bool operator ==(const Vector2& value) const;
		bool operator !=(const Vector2& value) const;
        float Dot(const Vector2& v) const { return x * v.x + y * v.y; }
		String ToString() const;
		float Magnitude() const;
		float SqrMagnitude() const;

		float x;
		float y;
	};
}
