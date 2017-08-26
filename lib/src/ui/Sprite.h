#pragma once

#include "Object.h"
#include "math/Rect.h"
#include "math/Vector2.h"
#include "math/Vector4.h"
#include "graphics/Texture2D.h"

namespace Viry3D
{
	class Sprite : public Object
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