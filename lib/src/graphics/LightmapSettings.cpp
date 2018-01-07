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

#include "LightmapSettings.h"
#include "Application.h"
#include "io/File.h"
#include "io/MemoryStream.h"

namespace Viry3D
{
	Vector<Ref<Texture2D>> LightmapSettings::m_lightmaps;

	const Texture2D* LightmapSettings::GetLightmap(int index)
	{
		if (index >= 0 && index < m_lightmaps.Size())
		{
			return m_lightmaps[index].get();
		}

		return NULL;
	}
}
