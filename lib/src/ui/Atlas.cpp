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
