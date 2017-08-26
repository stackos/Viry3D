#pragma once

#include "Sprite.h"

namespace Viry3D
{
	class Atlas : public Object
	{
	public:
		static Ref<Atlas> Create();

		void AddSprite(String name, const Ref<Sprite>& sprite);
		void RemoveSprite(String name);
		Ref<Sprite> GetSprite(String name);
		void SetTexture(const Ref<Texture>& texture) { m_texture = texture; }
		const Ref<Texture>& GetTexture() const { return m_texture; }

	private:
		Atlas();

		Map<String, Ref<Sprite>> m_sprites;
		Ref<Texture> m_texture;
	};
}