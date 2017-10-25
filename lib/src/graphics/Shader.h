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
#include "vulkan/ShaderVulkan.h"
#elif VR_GLES
#include "gles/ShaderGLES.h"
#endif

#include "Object.h"
#include "XMLShader.h"
#include "container/Map.h"
#include "thread/Thread.h"

namespace Viry3D
{
	class Texture2D;

#if VR_VULKAN
	class Shader: public ShaderVulkan
	{
		friend class ShaderVulkan;
#elif VR_GLES
	class Shader: public ShaderGLES
	{
		friend class ShaderGLES;
#endif
	public:
		static void Init();
		static void Deinit();
		static void ClearAllPipelines();
		static Ref<Shader> Find(const String& name);
		static Ref<Shader> ReplaceToShadowMapShader(const Ref<Shader>& shader);
		static const Ref<Texture2D>& GetDefaultTexture(const String& name);

		int GetQueue() const;

	private:
		Shader(const String& name);

		static Map<String, Ref<Shader>> m_shaders;
		static Mutex m_mutex;
		static Map<String, Ref<Texture2D>> m_default_textures;
		XMLShader m_xml;
	};
}
