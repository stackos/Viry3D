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