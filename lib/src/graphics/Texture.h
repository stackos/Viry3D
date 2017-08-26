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

#if VR_VULKAN
#include "vulkan/TextureVulkan.h"
#elif VR_GLES
#include "gles/TextureGLES.h"
#endif

#include "TextureWrapMode.h"
#include "FilterMode.h"

namespace Viry3D
{
#if VR_VULKAN
	class Texture: public TextureVulkan
	{
		friend class TextureVulkan;
#elif VR_GLES
	class Texture: public TextureGLES
	{
		friend class TextureGLES;
#endif
	public:
		Texture()
		{
			SetName("Texture");
			SetWidth(0);
			SetHeight(0);
		}
		int GetWidth() const { return m_width; }
		int GetHeight() const { return m_height; }
		TextureWrapMode GetWrapMode() const { return m_wrap_mode; }
		void SetWrapMode(TextureWrapMode mode) { m_wrap_mode = mode; }
		FilterMode GetFilterMode() const { return m_filter_mode; }
		void SetFilterMode(FilterMode mode) { m_filter_mode = mode; }

	protected:
		void SetWidth(int witdh) { m_width = witdh; }
		void SetHeight(int height) { m_height = height; }

	protected:
		int m_width;
		int m_height;
		TextureWrapMode m_wrap_mode;
		FilterMode m_filter_mode;
	};
}
