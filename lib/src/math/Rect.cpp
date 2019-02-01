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

#include "Rect.h"
#include "Mathf.h"

namespace Viry3D
{
    Rect Rect::Max(const Rect& a, const Rect& b)
    {
        float x_min = Mathf::Min(a.x, b.x);
        float y_min = Mathf::Min(a.y, b.y);
        float x_max = Mathf::Max(a.x + a.width, b.x + b.width);
        float y_max = Mathf::Max(a.y + a.height, b.y + b.height);
        return Rect(x_min, y_min, x_max - x_min, y_max - y_min);
    }

    Rect Rect::Min(const Rect& a, const Rect& b)
    {
        float x_min = Mathf::Max(a.x, b.x);
        float y_min = Mathf::Max(a.y, b.y);
        float x_max = Mathf::Min(a.x + a.width, b.x + b.width);
        float y_max = Mathf::Min(a.y + a.height, b.y + b.height);
        return Rect(x_min, y_min, Mathf::Max(x_max - x_min, 0.0f), Mathf::Max(y_max - y_min, 0.0f));
    }

	bool Rect::operator ==(const Rect& r) const
	{
		return Mathf::FloatEqual(x, r.x) &&
			Mathf::FloatEqual(y, r.y) &&
			Mathf::FloatEqual(width, r.width) &&
			Mathf::FloatEqual(height, r.height);
	}

	bool Rect::operator !=(const Rect& r) const
	{
		return !(*this == r);
	}
}
