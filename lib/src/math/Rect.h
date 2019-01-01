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
	struct Rect
	{
		explicit Rect(float x = 0, float y = 0, float width = 0, float height = 0):
            x(x),
            y(y),
			width(width),
			height(height)
		{
		}

		void Set(float x, float y, float width, float height)
		{
			this->x = x;
			this->y = y;
			this->width = width;
			this->height = height;
		}

		bool operator ==(const Rect &r) const;
		bool operator !=(const Rect &r) const;

		float x;
		float y;
		float width;
		float height;
	};
}
