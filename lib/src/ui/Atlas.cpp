#include "Atlas.h"

namespace Viry3D
{
	Ref<Atlas> Atlas::Create()
	{
		return Ref<Atlas>(new Atlas());
	}

	Atlas::Atlas()
	{
	}

	void Atlas::AddSprite(String name, const Ref<Sprite>& sprite)
	{
		m_sprites.Add(name, sprite);
	}

	void Atlas::RemoveSprite(String name)
	{
		m_sprites.Remove(name);
	}

	Ref<Sprite> Atlas::GetSprite(String name)
	{
		return m_sprites[name];
	}
}