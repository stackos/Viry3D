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

#include "Bounds.h"

namespace Viry3D
{
    Bounds::Bounds():
        m_min(0, 0, 0),
        m_max(0, 0, 0)
    {
        
    }

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
