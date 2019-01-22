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

#include "string/String.h"

namespace Viry3D
{
	struct Color
	{
		static const Color& White();
		static const Color& Black();
		static const Color& Red();
		static const Color& Green();
		static const Color& Blue();

		static Color Lerp(const Color &from, const Color &to, float t, bool clamp_01 = true);
        static Color Parse(const String& str);

		Color(float r = 1, float g = 1, float b = 1, float a = 1);
		bool operator ==(const Color &c) const;
		bool operator !=(const Color &c) const;
		Color operator *(const Color &c) const;
		Color &operator *=(const Color &c);
		Color operator *(float v) const;
		Color operator /(float v) const;

		float r;
		float g;
		float b;
		float a;
	};
}
