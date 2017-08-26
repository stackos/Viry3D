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
	class Texture : public TextureVulkan
	{
		friend class TextureVulkan;
#elif VR_GLES
	class Texture : public TextureGLES
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
		TextureWrapMode::Enum GetWrapMode() const { return m_wrap_mode; }
		void SetWrapMode(TextureWrapMode::Enum mode) { m_wrap_mode = mode; }
		FilterMode::Enum GetFilterMode() const { return m_filter_mode; }
		void SetFilterMode(FilterMode::Enum mode) { m_filter_mode = mode; }

	protected:
		void SetWidth(int witdh) { m_width = witdh; }
		void SetHeight(int height) { m_height = height; }

	protected:
		int m_width;
		int m_height;
		TextureWrapMode::Enum m_wrap_mode;
		FilterMode::Enum m_filter_mode;
	};
}