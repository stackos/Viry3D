#pragma once

#include "string/String.h"

namespace Viry3D
{
	struct Vector3;

	struct Vector2
	{
		explicit Vector2(float x = 0, float y = 0): x(x), y(y) { }
		Vector2(const Vector3& v3);
		Vector2 operator *(float value) const;
		Vector2& operator *=(float value);
		Vector2 operator +(const Vector2& value) const;
		Vector2 operator -(const Vector2& value) const;
		bool operator ==(const Vector2& value) const;
		bool operator !=(const Vector2& value) const;
		String ToString() const;
		float Magnitude() const;
		float SqrMagnitude() const;

		float x;
		float y;
	};
}