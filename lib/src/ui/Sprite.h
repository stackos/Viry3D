/*
* Viry3D
* Copyright 2014-2017 by Stack - stackos@qq.com
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

#include "Object.h"
#include "math/Rect.h"
#include "math/Vector2.h"
#include "math/Vector4.h"
#include "graphics/Texture2D.h"

namespace Viry3D
{
	class Sprite: public Object
	{
	public:
		static Ref<Sprite> Create(const Rect& rect, const Vector4& border);

		const Rect& GetRect() const { return m_rect; }
		const Vector4& GetBorder() const { return m_border; }

	private:
		Sprite();

		Rect m_rect;
		Vector4 m_border;
	};
}
