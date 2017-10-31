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

#include "Texture.h"
#include "TextureFormat.h"

namespace Viry3D
{
	enum class CubemapFace
	{
		Unknown,
		PositiveX,
		NegativeX,
		PositiveY,
		NegativeY,
		PositiveZ,
		NegativeZ,
	};

	class Cubemap: public Texture
	{
	public:
		static Ref<Cubemap> Create(
			int size,
			TextureFormat format,
			TextureWrapMode wrap_mode,
			FilterMode filter_mode,
			bool mipmap);

		TextureFormat GetFormat() const { return m_format; }
		bool IsMipmap() const { return m_mipmap; }
		void SetPixels(const ByteBuffer& colors, CubemapFace face, int mip_level = 0);
		void Apply(bool update_mipmaps = true, bool make_no_longer_readable = false);

	private:
		Cubemap();

	private:
		TextureFormat m_format;
		bool m_mipmap;
	};
}
