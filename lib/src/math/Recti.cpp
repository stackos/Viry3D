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

#include "Recti.h"
#include "Mathf.h"

namespace Viry3D
{
    Recti Recti::Max(const Recti& a, const Recti& b)
    {
        int x_min = Mathf::Min(a.x, b.x);
        int y_min = Mathf::Min(a.y, b.y);
        int x_max = Mathf::Max(a.x + a.w, b.x + b.w);
        int y_max = Mathf::Max(a.y + a.h, b.y + b.h);
        return Recti(x_min, y_min, x_max - x_min, y_max - y_min);
    }

    Recti Recti::Min(const Recti& a, const Recti& b)
    {
        int x_min = Mathf::Max(a.x, b.x);
        int y_min = Mathf::Max(a.y, b.y);
        int x_max = Mathf::Min(a.x + a.w, b.x + b.w);
        int y_max = Mathf::Min(a.y + a.h, b.y + b.h);
        return Recti(x_min, y_min, Mathf::Max(x_max - x_min, 0), Mathf::Max(y_max - y_min, 0));
    }

	bool Recti::operator ==(const Recti& r) const
	{
		return x == r.x && y == r.y && w == r.w && h == r.h;
	}

	bool Recti::operator !=(const Recti& r) const
	{
		return !(*this == r);
	}
}
