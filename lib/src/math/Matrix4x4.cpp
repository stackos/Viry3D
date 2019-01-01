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

#include "Matrix4x4.h"
#include "Mathf.h"
#include <sstream>

namespace Viry3D
{
	Matrix4x4 Matrix4x4::Identity()
	{
		static Matrix4x4 s_identity = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		};
		return s_identity;
	}

	Matrix4x4::Matrix4x4(const float* ms):
		m00(ms[0]),
		m01(ms[1]),
		m02(ms[2]),
		m03(ms[3]),
		m10(ms[4]),
		m11(ms[5]),
		m12(ms[6]),
		m13(ms[7]),
		m20(ms[8]),
		m21(ms[9]),
		m22(ms[10]),
		m23(ms[11]),
		m30(ms[12]),
		m31(ms[13]),
		m32(ms[14]),
		m33(ms[15])
	{
	}

	void Matrix4x4::SetRow(int row, const Vector4& v)
	{
		float* ptr = (float*) this;

		ptr[row * 4 + 0] = v.x;
		ptr[row * 4 + 1] = v.y;
		ptr[row * 4 + 2] = v.z;
		ptr[row * 4 + 3] = v.w;
	}

	Vector4 Matrix4x4::GetRow(int row)
	{
		float* ptr = (float*) this;

		return Vector4(
			ptr[row * 4 + 0],
			ptr[row * 4 + 1],
			ptr[row * 4 + 2],
			ptr[row * 4 + 3]);
	}

	void Matrix4x4::SetColumn(int row, const Vector4& v)
	{
		float* ptr = (float*) this;

		ptr[0 * 4 + row] = v.x;
		ptr[1 * 4 + row] = v.y;
		ptr[2 * 4 + row] = v.z;
		ptr[3 * 4 + row] = v.w;
	}

	Vector4 Matrix4x4::GetColumn(int row)
	{
		float* ptr = (float*) this;

		return Vector4(
			ptr[0 * 4 + row],
			ptr[1 * 4 + row],
			ptr[2 * 4 + row],
			ptr[3 * 4 + row]);
	}

	String Matrix4x4::ToString() const
	{
		std::stringstream ss;
		ss << m00 << ' ' << m01 << ' ' << m02 << ' ' << m03 << std::endl;
		ss << m10 << ' ' << m11 << ' ' << m12 << ' ' << m13 << std::endl;
		ss << m20 << ' ' << m21 << ' ' << m22 << ' ' << m23 << std::endl;
		ss << m30 << ' ' << m31 << ' ' << m32 << ' ' << m33 << std::endl;
		auto str = ss.str();
		return str.c_str();
	}

	Matrix4x4 Matrix4x4::Transpose() const
	{
		Matrix4x4 m(*this);

		m.m01 = m10;
		m.m02 = m20;
		m.m03 = m30;
		m.m10 = m01;
		m.m12 = m21;
		m.m13 = m31;
		m.m20 = m02;
		m.m21 = m12;
		m.m23 = m32;
		m.m30 = m03;
		m.m31 = m13;
		m.m32 = m23;

		return m;
	}

	Matrix4x4 Matrix4x4::Translation(const Vector3& t)
	{
		Matrix4x4 m = Identity();

		m.m03 = t.x;
		m.m13 = t.y;
		m.m23 = t.z;

		return m;
	}

    Matrix4x4 Matrix4x4::Rotation(const Quaternion& r)
    {
        Matrix4x4 m = Identity();

        m.m00 = 1 - 2 * r.y * r.y - 2 * r.z * r.z;
        m.m10 = 2 * r.x * r.y + 2 * r.w * r.z;
        m.m20 = 2 * r.x * r.z - 2 * r.w * r.y;

        m.m01 = 2 * r.x * r.y - 2 * r.w * r.z;
        m.m11 = 1 - 2 * r.x * r.x - 2 * r.z * r.z;
        m.m21 = 2 * r.y * r.z + 2 * r.w * r.x;

        m.m02 = 2 * r.x * r.z + 2 * r.w * r.y;
        m.m12 = 2 * r.y * r.z - 2 * r.w * r.x;
        m.m22 = 1 - 2 * r.x * r.x - 2 * r.y * r.y;

        return m;
    }

	Matrix4x4 Matrix4x4::Scaling(const Vector3& s)
	{
		Matrix4x4 m = Identity();

		m.m00 = s.x;
		m.m11 = s.y;
		m.m22 = s.z;

		return m;
	}

	Matrix4x4 Matrix4x4::TRS(const Vector3& t, const Quaternion& r, const Vector3& s)
	{
		Matrix4x4 mt = Translation(t);
		Matrix4x4 mr = Rotation(r);
		Matrix4x4 ms = Scaling(s);

		return mt * mr * ms;
	}

	Matrix4x4 Matrix4x4::LookTo(const Vector3& eye_position, const Vector3& to_direction, const Vector3& up_direction)
	{
		Matrix4x4 m = Matrix4x4::Identity();

		Vector3 zaxis(-to_direction);
		zaxis.Normalize();

		Vector3 xaxis = zaxis * up_direction;
		xaxis.Normalize();

		Vector3 yaxis = xaxis * zaxis;

		m.m00 = xaxis.x;	m.m01 = xaxis.y;	m.m02 = xaxis.z;	m.m03 = -xaxis.Dot(eye_position);
		m.m10 = yaxis.x;	m.m11 = yaxis.y;	m.m12 = yaxis.z;	m.m13 = -yaxis.Dot(eye_position);
		m.m20 = zaxis.x;	m.m21 = zaxis.y;	m.m22 = zaxis.z;	m.m23 = -zaxis.Dot(eye_position);
		m.m30 = 0;			m.m31 = 0;			m.m32 = 0;			m.m33 = 1.0f;

		return m;
	}

	Matrix4x4 Matrix4x4::Perspective(float fov, float aspect, float near, float far)
	{
		Matrix4x4 m = Matrix4x4::Identity();

		float scale_y = 1 / tan(Mathf::Deg2Rad * fov / 2);
		float scale_x = scale_y / aspect;

		m.m00 = scale_x;
		m.m11 = scale_y;
		m.m22 = (near + far) / (near - far);
		m.m23 = 2 * near * far / (near - far);
		m.m32 = -1.0f;
		m.m33 = 0;

		return m;
	}

	Matrix4x4 Matrix4x4::Ortho(float left, float right, float bottom, float top, float near, float far)
	{
		Matrix4x4 m = Matrix4x4::Identity();

		float r_l = 1 / (right - left);
		float t_b = 1 / (top - bottom);
		float n_f = 1 / (near - far);

		m.m00 = 2 * r_l;
		m.m03 = -(right + left) * r_l;
		m.m11 = 2 * t_b;
		m.m13 = -(top + bottom) * t_b;
#if 1
		//cvv -1~1
		m.m22 = 2 * n_f;
		m.m23 = (near + far) * n_f;
#else
		//cvv 0~1
		m.m22 = n_f;
		m.m23 = near * n_f;
#endif

		return m;
	}

	Matrix4x4 Matrix4x4::operator *(const Matrix4x4& mat) const
	{
		Matrix4x4 m = Identity();

        m.m00 = m00 * mat.m00 + m01 * mat.m10 + m02 * mat.m20 + m03 * mat.m30;
        m.m01 = m00 * mat.m01 + m01 * mat.m11 + m02 * mat.m21 + m03 * mat.m31;
        m.m02 = m00 * mat.m02 + m01 * mat.m12 + m02 * mat.m22 + m03 * mat.m32;
        m.m03 = m00 * mat.m03 + m01 * mat.m13 + m02 * mat.m23 + m03 * mat.m33;

        m.m10 = m10 * mat.m00 + m11 * mat.m10 + m12 * mat.m20 + m13 * mat.m30;
        m.m11 = m10 * mat.m01 + m11 * mat.m11 + m12 * mat.m21 + m13 * mat.m31;
        m.m12 = m10 * mat.m02 + m11 * mat.m12 + m12 * mat.m22 + m13 * mat.m32;
        m.m13 = m10 * mat.m03 + m11 * mat.m13 + m12 * mat.m23 + m13 * mat.m33;

        m.m20 = m20 * mat.m00 + m21 * mat.m10 + m22 * mat.m20 + m23 * mat.m30;
        m.m21 = m20 * mat.m01 + m21 * mat.m11 + m22 * mat.m21 + m23 * mat.m31;
        m.m22 = m20 * mat.m02 + m21 * mat.m12 + m22 * mat.m22 + m23 * mat.m32;
        m.m23 = m20 * mat.m03 + m21 * mat.m13 + m22 * mat.m23 + m23 * mat.m33;

        m.m30 = m30 * mat.m00 + m31 * mat.m10 + m32 * mat.m20 + m33 * mat.m30;
        m.m31 = m30 * mat.m01 + m31 * mat.m11 + m32 * mat.m21 + m33 * mat.m31;
        m.m32 = m30 * mat.m02 + m31 * mat.m12 + m32 * mat.m22 + m33 * mat.m32;
        m.m33 = m30 * mat.m03 + m31 * mat.m13 + m32 * mat.m23 + m33 * mat.m33;

		return m;
	}

	Vector4 Matrix4x4::operator *(const Vector4& v) const
	{
		float x = v.x;
		float y = v.y;
		float z = v.z;
		float w = v.w;
		float vx, vy, vz, vw;

		vx = x*m00 + y*m01 + z*m02 + w*m03;
		vy = x*m10 + y*m11 + z*m12 + w*m13;
		vz = x*m20 + y*m21 + z*m22 + w*m23;
		vw = x*m30 + y*m31 + z*m32 + w*m33;

		return Vector4(vx, vy, vz, vw);
	}

	Vector3 Matrix4x4::MultiplyPoint(const Vector3& v) const
	{
		float x = v.x;
		float y = v.y;
		float z = v.z;
		float vx, vy, vz, vw;

		vx = x*m00 + y*m01 + z*m02 + m03;
		vy = x*m10 + y*m11 + z*m12 + m13;
		vz = x*m20 + y*m21 + z*m22 + m23;
		vw = x*m30 + y*m31 + z*m32 + m33;

		return Vector3(vx / vw, vy / vw, vz / vw);
	}

	Vector3 Matrix4x4::MultiplyPoint3x4(const Vector3& v) const
	{
		float x = v.x;
		float y = v.y;
		float z = v.z;
		float vx, vy, vz;

		vx = x*m00 + y*m01 + z*m02 + m03;
		vy = x*m10 + y*m11 + z*m12 + m13;
		vz = x*m20 + y*m21 + z*m22 + m23;

		return Vector3(vx, vy, vz);
	}

	Vector3 Matrix4x4::MultiplyDirection(const Vector3& v) const
	{
		float x = v.x;
		float y = v.y;
		float z = v.z;
		float vx, vy, vz;

		vx = x*m00 + y*m01 + z*m02;
		vy = x*m10 + y*m11 + z*m12;
		vz = x*m20 + y*m21 + z*m22;

		return Vector3(vx, vy, vz);
	}

#define SWAP(a, b, temp) \
	do \
	{ \
		temp = a; \
		a = b; \
		b = temp; \
	} while(false)

	Matrix4x4 Matrix4x4::Inverse() const
	{
		Matrix4x4 ret(*this);
		float* mat = (float*) & ret;
		int is[4];
		int js[4];

		for (int i = 0; i < 4; ++i)
		{
			float max = 0.0f;

			for (int j = i; j < 4; ++j)
			{
				for (int k = i; k < 4; ++k)
				{
					const float f = fabs(mat[j * 4 + k]);
					if (f > max)
					{
						max = f;
						is[i] = j;
						js[i] = k;
					}
				}
			}

			if (max < 0.0001f)
			{
				return ret;
			}

			if (is[i] != i)
			{
				float temp;

				SWAP(mat[is[i] * 4 + 0], mat[i * 4 + 0], temp);
				SWAP(mat[is[i] * 4 + 1], mat[i * 4 + 1], temp);
				SWAP(mat[is[i] * 4 + 2], mat[i * 4 + 2], temp);
				SWAP(mat[is[i] * 4 + 3], mat[i * 4 + 3], temp);
			}

			if (js[i] != i)
			{
				float temp;

				SWAP(mat[0 * 4 + js[i]], mat[0 * 4 + i], temp);
				SWAP(mat[1 * 4 + js[i]], mat[1 * 4 + i], temp);
				SWAP(mat[2 * 4 + js[i]], mat[2 * 4 + i], temp);
				SWAP(mat[3 * 4 + js[i]], mat[3 * 4 + i], temp);
			}

			float& key = mat[i * 4 + i];
			key = 1.0f / key;

			for (int j = 0; j < 4; ++j)
			{
				if (j != i)
				{
					mat[i * 4 + j] *= key;
				}
			}

			for (int j = 0; j < 4; ++j)
			{
				if (j != i)
				{
					for (int k = 0; k < 4; ++k)
					{
						if (k != i)
						{
							mat[j * 4 + k] -= mat[i * 4 + k] * mat[j * 4 + i];
						}
					}
				}
			}

			for (int j = 0; j < 4; ++j)
			{
				if (j != i)
				{
					mat[j * 4 + i] *= -key;
				}
			}
		}

		for (int i = 3; i >= 0; --i)
		{
			if (js[i] != i)
			{
				float temp;

				SWAP(mat[js[i] * 4 + 0], mat[i * 4 + 0], temp);
				SWAP(mat[js[i] * 4 + 1], mat[i * 4 + 1], temp);
				SWAP(mat[js[i] * 4 + 2], mat[i * 4 + 2], temp);
				SWAP(mat[js[i] * 4 + 3], mat[i * 4 + 3], temp);
			}

			if (is[i] != i)
			{
				float temp;

				SWAP(mat[0 * 4 + is[i]], mat[0 * 4 + i], temp);
				SWAP(mat[1 * 4 + is[i]], mat[1 * 4 + i], temp);
				SWAP(mat[2 * 4 + is[i]], mat[2 * 4 + i], temp);
				SWAP(mat[3 * 4 + is[i]], mat[3 * 4 + i], temp);
			}
		}

		return ret;
	}
}
