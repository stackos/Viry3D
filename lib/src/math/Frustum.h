#pragma once

#include "Matrix4x4.h"
#include "Vector3.h"

namespace Viry3D
{
	struct ContainsResult
    {
        enum Enum
        {
            In,
            Out,
            Cross
        };
    };

	class Frustum
	{
	public:
		Frustum() { }

		//
        // 摘要:
        //     ///
        //     构建一个FrustumBounds, 投影类型为perspective.
        //     ///
        //
        // 参数:
        //   mat:
        //     wvp matrix, check pos in model space.
        //     vp matrix, check pos in world space.
        //     p matrix, check pos in view space.
		Frustum(const Matrix4x4& mat);

		//
        // 摘要:
        //     ///
        //     构建一个FrustumBounds, 投影类型为orthographic, in view space.
        //     ///
        Frustum(float left, float right, float bottom, float top, float near, float far);

		ContainsResult::Enum ContainsPoint(const Vector3& point) const;
		ContainsResult::Enum ContainsSphere(const Vector3& center, float radius) const;
        ContainsResult::Enum ContainsBounds(const Vector3& min, const Vector3& max) const;
        ContainsResult::Enum ContainsPoints(const Vector<Vector3>& points, const Matrix4x4* matrix) const;
        float DistanceToPlane(const Vector3& point, int plane_index) const;

	private:
		void NormalizePlanes();

		Vector4 m_planes[6];
	};
}