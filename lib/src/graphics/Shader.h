/*
* Viry3D
* Copyright 2014-2019 by Stack - stackos@qq.com
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

#include "Object.h"
#include "container/Vector.h"
#include "container/List.h"
#include "container/Map.h"
#include "private/backend/DriverApi.h"

#if VR_VULKAN
#include <bluevk/BlueVK.h>
#endif

namespace Viry3D
{
    class Shader : public Object
    {
    public:
		enum class BindingPoints
		{
			PerView = 0,
			PerRenderer = 1,
			PerRendererBones = 2,
			Lights = 3,
			PostProcess = 4,
			PerMaterialInstance = 5,
			Count = 6,
		};

		enum class Queue
		{
			Background = 1000,
			Geometry = 2000,
			AlphaTest = 2450,
			Transparent = 3000,
			Overlay = 4000,
		};

		struct Pass
		{
			String vs;
			String fs;
			int queue = (int) Queue::Geometry;
			filament::backend::PipelineState pipeline;
		};

#if VR_VULKAN
		static void GlslToSpirv(const String& glsl, VkShaderStageFlagBits shader_type, Vector<unsigned int>& spirv);
#endif

        static void Init();
        static void Done();
		static Ref<Shader> Find(const String& name, const Vector<String>& keywords = Vector<String>());

        Shader(const String& name);
        virtual ~Shader();

	private:
		void Load(const String& src, const List<String>& keywords);
		void Compile();

	private:
		static Map<String, Ref<Shader>> m_shaders;
		List<String> m_keywords;
		Vector<Pass> m_passes;
		int m_queue;
    };
}
