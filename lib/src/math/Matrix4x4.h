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
#include "Vector4.h"
#include "Quaternion.h"

namespace Viry3D
{
	//column major
	struct Matrix4x4
	{
		Matrix4x4() { }
		Matrix4x4(
			float m00, float m01, float m02, float m03,
			float m10, float m11, float m12, float m13,
			float m20, float m21, float m22, float m23,
			float m30, float m31, float m32, float m33):
			m00(m00), m01(m01), m02(m02), m03(m03),
			m10(m10), m11(m11), m12(m12), m13(m13),
			m20(m20), m21(m21), m22(m22), m23(m23),
			m30(m30), m31(m31), m32(m32), m33(m33)
		{
		}
		Matrix4x4 operator *(const Matrix4x4& mat) const;
		Vector4 operator *(const Vector4& v) const;
		Vector3 MultiplyPoint(const Vector3& v) const;
		Vector3 MultiplyPoint3x4(const Vector3& v) const;
		Vector3 MultiplyDirection(const Vector3& v) const;
		Matrix4x4 Inverse() const;
		Matrix4x4 Transpose() const;
		String ToString() const;
		void SetRow(int row, const Vector4& v);
		Vector4 GetRow(int row);
		void SetColumn(int row, const Vector4& v);
		Vector4 GetColumn(int row);

		static Matrix4x4 Identity();
		static Matrix4x4 Translation(const Vector3& t);
		static Matrix4x4 Rotation(const Quaternion& r);
		static Matrix4x4 Scaling(const Vector3& s);
		static Matrix4x4 TRS(const Vector3& t, const Quaternion& r, const Vector3& s);
        static Matrix4x4 LookTo(const Vector3& eye_position, const Vector3& to_direction, const Vector3& up_direction);
		static Matrix4x4 Perspective(float fov, float aspect, float zNear, float zFar);
		static Matrix4x4 Ortho(float left, float right, float bottom, float top, float zNear, float zFar);

		float m00;
		float m01;
		float m02;
		float m03;
		float m10;
		float m11;
		float m12;
		float m13;
		float m20;
		float m21;
		float m22;
		float m23;
		float m30;
		float m31;
		float m32;
		float m33;

	private:
		Matrix4x4(const float* ms);
	};
}
