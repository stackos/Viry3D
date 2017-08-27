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

#include "math/Vector2.h"
#include "math/Rect.h"

namespace Viry3D
{
	class UIRect
	{
	public:
		virtual void SetAnchors(const Vector2& min, const Vector2& max);
		virtual void SetOffsets(const Vector2& min, const Vector2& max);
		virtual void SetPivot(const Vector2& pivot);
		virtual void SetSize(const Vector2& size);
		const Vector2& GetAnchorMin() const { return m_anchor_min; }
		const Vector2& GetAnchorMax() const { return m_anchor_max; }
		const Vector2& GetOffsetMin() const { return m_offset_min; }
		const Vector2& GetOffsetMax() const { return m_offset_max; }
		const Vector2& GetPivot() const { return m_pivot; }
		void OnAnchor();

	protected:
		UIRect();
		Ref<UIRect> GetParentRect() const;
		Vector2 GetSize() const;

		Vector2 m_anchor_min;
		Vector2 m_anchor_max;
		Vector2 m_offset_min;
		Vector2 m_offset_max;
		Vector2 m_pivot;
		bool m_dirty;
	};
}
