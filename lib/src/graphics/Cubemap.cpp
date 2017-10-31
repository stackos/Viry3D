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

#include "Cubemap.h"

namespace Viry3D
{
	Ref<Cubemap> Cubemap::Create(
		int size,
		TextureFormat format,
		TextureWrapMode wrap_mode,
		FilterMode filter_mode,
		bool mipmap)
	{
		Ref<Cubemap> cubemap = Ref<Cubemap>(new Cubemap());
		cubemap->SetWidth(size);
		cubemap->SetHeight(size);
		cubemap->SetWrapMode(wrap_mode);
		cubemap->SetFilterMode(filter_mode);
		cubemap->m_format = format;
		cubemap->m_mipmap = mipmap;

		cubemap->CreateCubemap();

		return cubemap;
	}

	Cubemap::Cubemap():
		m_format(TextureFormat::RGBA32)
	{
		SetName("Cubemap");
	}

	void Cubemap::SetPixels(const ByteBuffer& colors, CubemapFace face, int mip_level)
	{
		if (m_colors.Empty())
		{
			m_colors.Resize(this->GetMipmapCount());
		}

		m_colors[mip_level][(int) face] = colors;
	}

	void Cubemap::Apply(bool update_mipmaps, bool make_no_longer_readable)
	{
		
	}
}
