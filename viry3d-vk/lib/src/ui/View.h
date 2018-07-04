/*
* Viry3D
* Copyright 2014-2018 by Stack - stackos@qq.com
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

#include "container/Vector.h"
#include "graphics/Color.h"
#include "math/Vector2.h"

namespace Viry3D
{
	class CanvaRenderer;

	enum class ViewAlignment
	{
		HorizontalLeft = 0x00000001,
		HorizontalCenter = 0x00000002,
		HorizontalRight = 0x00000004,
		VerticalTop = 0x00000010,
		VerticalCenter = 0x00000020,
		VerticalBottom = 0x00000040,
	};

	class View
	{
	public:
		View();
		virtual ~View();
		void OnAddToCanvas(CanvaRenderer* canvas);
		void OnRemoveFromCanvas(CanvaRenderer* canvas);
		const Color& GetColor() const { return m_color; }
		void SetColor(const Color& color);
		int GetAlignment() const { return m_alignment; }
		void SetAlignment(int alignment);
		const Vector2& GetPivot() const { return m_pivot; }
		void SetPivot(const Vector2& pivot);
		const Vector2& GetSize() const { return m_size; }
		void SetSize(const Vector2& size);
		const Vector2& GetOffset() const { return m_offset; }
		void SetOffset(const Vector2& offset);

	private:
		CanvaRenderer* m_canvas;
		Vector<Ref<View>> m_children;
		Color m_color;
		int m_alignment;
		Vector2 m_pivot;
		Vector2 m_size;
		Vector2 m_offset;
	};
}
