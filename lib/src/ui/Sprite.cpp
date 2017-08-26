#include "Sprite.h"

namespace Viry3D
{
	Ref<Sprite> Sprite::Create(const Rect& rect, const Vector4& border)
	{
		Ref<Sprite> sprite;

		sprite = Ref<Sprite>(new Sprite());
		sprite->m_rect = rect;
		sprite->m_border = border;

		return sprite;
	}

	Sprite::Sprite():
		m_rect(0, 0, 1, 1),
		m_border(0, 0, 0, 0)
	{
	}
}