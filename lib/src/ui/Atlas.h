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

#include "Sprite.h"

namespace Viry3D
{
	class Atlas: public Object
	{
	public:
		static Ref<Atlas> Create();

		void AddSprite(const String& name, const Ref<Sprite>& sprite);
		void RemoveSprite(const String& name);
		Ref<Sprite> GetSprite(const String& name);
		void SetTexture(const Ref<Texture>& texture) { m_texture = texture; }
		const Ref<Texture>& GetTexture() const { return m_texture; }

	private:
		Atlas();

		Map<String, Ref<Sprite>> m_sprites;
		Ref<Texture> m_texture;
	};
}
