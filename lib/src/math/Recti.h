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

namespace Viry3D
{
	struct Recti
	{
        static Recti Max(const Recti& a, const Recti& b);
        static Recti Min(const Recti& a, const Recti& b);

		explicit Recti(int x = 0, int y = 0, int w = 0, int h = 0):
            x(x),
            y(y),
			w(w),
			h(h)
		{
		}

		void Set(int x, int y, int w, int h)
		{
			this->x = x;
			this->y = y;
			this->w = w;
			this->h = h;
		}

		bool operator ==(const Recti &r) const;
		bool operator !=(const Recti &r) const;

		int x;
        int y;
        int w;
        int h;
	};
}
