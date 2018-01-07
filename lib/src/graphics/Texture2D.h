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
		static bool LoadImageData(const ByteBuffer& buffer, ByteBuffer& colors, int& width, int& height, TextureFormat& format);
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
        static Ref<Texture2D> CreateExternalTexture(int width, int height, TextureFormat format, bool mipmap, void* external_texture);
        void UpdateExternalTexture(void* external_texture);

		ByteBuffer& GetColors() { return m_colors; }
		void UpdateTexture(int x, int y, int w, int h, const ByteBuffer& colors);
		void EncodeToPNG(const String& file);
		TextureFormat GetFormat() const { return m_format; }

	private:
		Texture2D();

	private:
		TextureFormat m_format;
		ByteBuffer m_colors;
	};
}
