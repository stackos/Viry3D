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
	class Texture2D: public Texture
	{
	public:
		//
		//	线程安全, 可在子线程中异步加载
		//
		static Ref<Texture2D> LoadFromFile(const String& file,
			TextureWrapMode wrap_mode = TextureWrapMode::Clamp,
			FilterMode filter_mode = FilterMode::Bilinear,
			bool mipmap = false);
		//
		//	线程安全, 可在子线程中异步加载
		//
		static Ref<Texture2D> LoadFromData(const ByteBuffer& buffer,
			TextureWrapMode wrap_mode = TextureWrapMode::Clamp,
			FilterMode filter_mode = FilterMode::Bilinear,
			bool mipmap = false);
		//
		//	线程安全
		//
		static Ref<Texture2D> Create(
			int width,
			int height,
			TextureFormat format,
			TextureWrapMode wrap_mode,
			FilterMode filter_mode,
			bool mipmap,
			const ByteBuffer& colors);

		ByteBuffer GetColors() const { return m_colors; }
		void UpdateTexture(int x, int y, int w, int h, const ByteBuffer& colors);
		void EncodeToPNG(const String& file);
		TextureFormat GetFormat() const { return m_format; }
		bool IsMipmap() const { return m_mipmap; }

	private:
		Texture2D();
		void SetFormat(TextureFormat format) { m_format = format; }
		void SetMipmap(bool mipmap) { m_mipmap = mipmap; }

	private:
		TextureFormat m_format;
		bool m_mipmap;
		ByteBuffer m_colors;
	};
}
