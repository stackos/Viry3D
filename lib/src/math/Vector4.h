#pragma once

#include "Mathf.h"
#include "Vector3.h"

namespace Viry3D
{
	struct Vector4
	{
		explicit Vector4(float x = 0, float y = 0, float z = 0, float w = 0):
			x(x), y(y), z(z), w(w)
		{
		}

		Vector4(const Vector3& v, float w = 0):
			x(v.x), y(v.y), z(v.z), w(w)
		{
		}

		bool operator ==(const Vector4& v) const
		{
			return
				Mathf::FloatEqual(x, v.x) &&
				Mathf::FloatEqual(y, v.y) &&
				Mathf::FloatEqual(z, v.z) &&
				Mathf::FloatEqual(w, v.w);
		}

		bool operator !=(const Vector4& v) const
		{
			return !(*this == v);
		}

		Vector4& operator *=(float v)
		{
			x *= v;
			y *= v;
			z *= v;
			w *= v;
			return *this;
		}

		Vector4 operator *(float v) const
		{
			return Vector4(x * v, y * v, z * v, w * v);
		}

		float& operator [](int index) const
		{
			return ((float *) this)[index];
		}

		static float Dot(const Vector4& v1, const Vector4& v2)
		{
			return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
		}

		Vector4 operator +(const Vector4& v) const
		{
			return Vector4(x + v.x, y + v.y, z + v.z, w + v.w);
		}

		Vector4 operator -(const Vector4& v) const
		{
			return Vector4(x - v.x, y - v.y, z - v.z, w - v.w);
		}

		float x;
		float y;
		float z;
		float w;
	};
}