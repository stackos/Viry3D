#include "LightmapSettings.h"
#include "Application.h"
#include "io/File.h"
#include "io/MemoryStream.h"

namespace Viry3D
{
	Vector<Ref<Texture2D>> LightmapSettings::m_lightmaps;

	const Texture2D* LightmapSettings::GetLightmap(int index)
	{
		if(index >= 0 && index < m_lightmaps.Size())
		{
			return m_lightmaps[index].get();
		}
		
		return NULL;
	}
}