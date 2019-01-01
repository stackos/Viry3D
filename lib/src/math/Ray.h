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

namespace Viry3D
{
	class Ray
    {
    public:
        Ray(const Vector3& origin, const Vector3& direction);
        const Vector3& GetOrigin() const { return m_origin; }
        void SetOrigin(const Vector3& origin) { m_origin = origin; }
		const Vector3& GetDirection() const { return m_direction; }
		void SetDirection(const Vector3& direction) { m_direction = Vector3::Normalize(direction); }
        Vector3 GetPoint(float distance) const;

	private:
        Vector3 m_origin;
		Vector3 m_direction;
    };
}
