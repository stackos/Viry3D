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

#include "string/String.h"
#include "Texture2D.h"

#define LIGHT_MAP_COUNT_MAX 8

namespace Viry3D
{
	class LightmapSettings
	{
	public:
		static void SetLightmaps(const Vector<Ref<Texture2D>>& maps) { m_lightmaps = maps; }
		static int GetLightmapCount() { return m_lightmaps.Size(); }
		static const Texture2D* GetLightmap(int index);
		static void Clear() { m_lightmaps.Clear(); }

	private:
		static Vector<Ref<Texture2D>> m_lightmaps;
	};
}
