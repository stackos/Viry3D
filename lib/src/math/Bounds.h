#pragma once

#include "Vector3.h"

namespace Viry3D
{
	class Bounds
	{
	public:
		Bounds(const Vector3& min, const Vector3& max);
		const Vector3& Min() const { return m_min; }
		const Vector3& Max() const { return m_max; }
		bool Contains(const Vector3& point) const;

	private:
		Vector3 m_min;
		Vector3 m_max;
	};
}