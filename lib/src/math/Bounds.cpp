#include "Bounds.h"

namespace Viry3D
{
	Bounds::Bounds(const Vector3& min, const Vector3& max):
		m_min(min),
		m_max(max)
	{
	}

	bool Bounds::Contains(const Vector3& point) const
	{
		return !(point.x < m_min.x || point.y < m_min.y || point.z < m_min.z ||
			point.x > m_max.x || point.y > m_max.y || point.z > m_max.z);
	}
}