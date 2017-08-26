#pragma once

#if VR_VULKAN
#include "vulkan/ShaderVulkan.h"
#elif VR_GLES
#include "gles/ShaderGLES.h"
#endif

#include "Object.h"
#include "XMLShader.h"
#include "container/Map.h"
#include <mutex>

namespace Viry3D
{
	class Texture2D;

#if VR_VULKAN
	class Shader : public ShaderVulkan
	{
		friend class ShaderVulkan;
#elif VR_GLES
	class Shader : public ShaderGLES
	{
		friend class ShaderGLES;
#endif
	public:
		static void Init();
		static void Deinit();
		static void ClearAllPipelines();
		static Ref<Shader> Find(String name);
		static const Ref<Texture2D>& GetDefaultTexture(const String& name);

		int GetQueue() const;

	private:
		Shader(String name);

		static Map<String, Ref<Shader>> m_shaders;
		static std::mutex m_mutex;
		static Map<String, Ref<Texture2D>> m_default_textures;
		XMLShader m_xml;
	};
}