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

#include "Quaternion.h"
#include "Mathf.h"
#include <sstream>

namespace Viry3D
{
	Quaternion Quaternion::Identity()
	{
		return Quaternion();
	}

	Quaternion::Quaternion(float x, float y, float z, float w):
		x(x), y(y), z(z), w(w)
	{
	}

	Vector3 Quaternion::ToEulerAngles() const
	{
		float rx = atan2(2 * (w * x + y * z), 1 - 2 * (x * x + y * y));
		float ry = asin(2 * (w * y - z * x));
		float rz = atan2(2 * (w * z + x * y), 1 - 2 * (z * z + y * y));

		return Vector3(rx, ry, rz) * Mathf::Rad2Deg;
	}

	String Quaternion::ToString() const
	{
		std::stringstream ss;
		ss << '(' << x << ',' << y << ',' << z << ',' << w << ')';
		auto str = ss.str();
		return str.c_str();
	}

	Quaternion Quaternion::operator *(const Quaternion& quat) const
	{
		float _x = w * quat.x + x * quat.w + y * quat.z - z * quat.y;
		float _y = w * quat.y + y * quat.w + z * quat.x - x * quat.z;
		float _z = w * quat.z + z * quat.w + x * quat.y - y * quat.x;
		float _w = w * quat.w - x * quat.x - y * quat.y - z * quat.z;

		return Quaternion(_x, _y, _z, _w);
	}

	Quaternion Quaternion::operator *(float v) const
	{
		return Quaternion(x * v, y * v, z * v, w * v);
	}

	Vector3 Quaternion::operator *(const Vector3& p) const
	{
		Quaternion p_ = *this * Quaternion(p.x, p.y, p.z, 0) * Inverse(*this);

		return Vector3(p_.x, p_.y, p_.z);
	}

	bool Quaternion::operator !=(const Quaternion& v) const
	{
		return !(*this == v);
	}

	bool Quaternion::operator ==(const Quaternion& v) const
	{
		return Mathf::FloatEqual(v.x, x) &&
			Mathf::FloatEqual(v.y, y) &&
			Mathf::FloatEqual(v.z, z) &&
			Mathf::FloatEqual(v.w, w);
	}

	Quaternion Quaternion::Inverse(const Quaternion& q)
	{
		return Quaternion(-q.x, -q.y, -q.z, q.w);
	}

	Quaternion Quaternion::AngleAxis(float angle, const Vector3& axis)
	{
		Vector3 v = Vector3::Normalize(axis);
		float cosv, sinv;

		cosv = cos(Mathf::Deg2Rad * angle * 0.5f);
		sinv = sin(Mathf::Deg2Rad * angle * 0.5f);

		float x = v.x * sinv;
		float y = v.y * sinv;
		float z = v.z * sinv;
		float w = cosv;

		return Quaternion(x, y, z, w);
	}

	Quaternion Quaternion::Euler(float x, float y, float z)
	{
		Quaternion around_x = AngleAxis(x, Vector3(1, 0, 0));
		Quaternion around_y = AngleAxis(y, Vector3(0, 1, 0));
		Quaternion around_z = AngleAxis(z, Vector3(0, 0, 1));

		return around_y * around_x * around_z;
	}

	Quaternion Quaternion::Lerp(const Quaternion& from, const Quaternion& to, float t)
	{
		Quaternion to_;

		if (from.Dot(to) < 0)
		{
			to_ = to * -1.0f;
		}
		else
		{
			to_ = to;
		}

		Quaternion lerp = Quaternion(
			Mathf::Lerp(from.x, to_.x, t, false),
			Mathf::Lerp(from.y, to_.y, t, false),
			Mathf::Lerp(from.z, to_.z, t, false),
			Mathf::Lerp(from.w, to_.w, t, false));
		lerp.Normalize();

		return lerp;
	}

	Quaternion Quaternion::SLerp(const Quaternion& from, const Quaternion& to, float t)
	{
		Quaternion to_;

		if (from.Dot(to) < 0)
		{
			to_ = to * -1.0f;
		}
		else
		{
			to_ = to;
		}

		Quaternion slerp;
		float t_ = 1 - t;
		float Wa, Wb;
		float theta = acos(from.x * to_.x + from.y * to_.y + from.z * to_.z + from.w * to_.w);
		float sn = sin(theta);
		if (!Mathf::FloatEqual(sn, 0))
		{
			float inv_sin = 1 / sn;
			Wa = sin(t_ * theta);
			Wb = sin(t * theta);
			slerp.x = (Wa * from.x + Wb * to_.x) * inv_sin;
			slerp.y = (Wa * from.y + Wb * to_.y) * inv_sin;
			slerp.z = (Wa * from.z + Wb * to_.z) * inv_sin;
			slerp.w = (Wa * from.w + Wb * to_.w) * inv_sin;
		}
		else
		{
			slerp = from;
		}

		slerp.Normalize();

		return slerp;
	}

	Quaternion Quaternion::FromToRotation(const Vector3& from_direction, const Vector3& to_direction)
	{
		Vector3 origin = Vector3::Normalize(from_direction);
		Vector3 fn = Vector3::Normalize(to_direction);

		if (fn != origin)
		{
			if (!Mathf::FloatEqual(fn.SqrMagnitude(), 0) && !Mathf::FloatEqual(origin.SqrMagnitude(), 0))
			{
				float deg = Vector3::Angle(origin, fn);
				Vector3 axis = origin * fn;

				if (axis == Vector3(0, 0, 0))
				{
					if (!Mathf::FloatEqual(origin.x, 0))
					{
						float x = -origin.y / origin.x;
						float y = 1;
						float z = 0;

						axis = Vector3(x, y, z);
					}
					else if (!Mathf::FloatEqual(origin.y, 0))
					{
						float y = -origin.z / origin.y;
						float x = 0;
						float z = 1;

						axis = Vector3(x, y, z);
					}
					else
					{
						float z = -origin.x / origin.z;
						float y = 0;
						float x = 1;

						axis = Vector3(x, y, z);
					}

					return Quaternion::AngleAxis(deg, axis);
				}
				else
				{
					return Quaternion::AngleAxis(deg, axis);
				}
			}
		}

		return Quaternion::Identity();
	}

	Quaternion Quaternion::LookRotation(const Vector3& forward, const Vector3& up)
	{
        Vector3 un = Vector3::Normalize(up);
        Vector3 fn = Vector3::Normalize(forward);

        Quaternion rot0 = Quaternion::FromToRotation(Vector3(0, 1, 0), un);
        Vector3 f = rot0 * Vector3(0, 0, 1);
        float deg = Vector3::Angle(f, fn);
        Quaternion rot1;
        Vector3 axis = f * fn;
        float d = axis.Dot(un);
        if (d > 0)
        {
            rot1 = Quaternion::AngleAxis(deg, up);
        }
        else
        {
            rot1 = Quaternion::AngleAxis(-deg, up);
        }
        return rot1 * rot0;
	}

	void Quaternion::Normalize()
	{
		float sqr_magnitude = x*x + y*y + z*z + w*w;
		if (!Mathf::FloatEqual(sqr_magnitude, 0))
		{
			float sq = sqrt(sqr_magnitude);

			float inv = 1.0f / sq;
			x = x * inv;
			y = y * inv;
			z = z * inv;
			w = w * inv;
		}
	}

	float Quaternion::Dot(const Quaternion& v) const
	{
		return x * v.x + y * v.y + z * v.z + w * v.w;
	}
}
