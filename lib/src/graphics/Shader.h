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

namespace Viry3D
{
    class Shader : public Object
    {
    public:
		enum class BindingPoint
		{
			PerView = 0,
			PerRenderer = 1,
			PerRendererBones = 2,
			PerMaterialVertex = 3,
			PerMaterialFragment = 4,
			PerLightVertex = 5,
			PerLightFragment = 6,

			Count = filament::backend::CONFIG_UNIFORM_BINDING_COUNT,
		};

		enum class AttributeLocation
		{
			Vertex = 0,
			Color = 1,
			UV = 2,
			UV2 = 3,
			Normal = 4,
			Tangent = 5,
			BoneWeights = 6,
			BoneIndices = 7,

			Count = filament::backend::MAX_VERTEX_ATTRIBUTE_COUNT
		};

		enum class Queue
		{
			Background = 1000,
			Geometry = 2000,
			AlphaTest = 2450,
			Transparent = 3000,
			Overlay = 4000,
		};

		enum class LightMode
		{
			None = 0,
			Forward = 1,
		};

		struct Member
		{
			String name;
			int offset;
			int size;
		};

		struct Uniform
		{
			String name;
			int binding;
			Vector<Member> members;
			int size;
		};

		struct Sampler
		{
			String name;
			int binding;
		};

		struct SamplerGroup
		{
			String name;
			int binding;
			Vector<Sampler> samplers;
		};

		struct Pass
		{
			String vs;
			String fs;
			int queue = (int) Queue::Geometry;
			LightMode light_mode = LightMode::None;
			Vector<Uniform> uniforms;
			Vector<SamplerGroup> samplers;
			filament::backend::PipelineState pipeline;
		};

        static void Init();
        static void Done();
		static Ref<Shader> Find(const String& name, const Vector<String>& keywords = Vector<String>(), bool light_add = false);

        virtual ~Shader();
		const List<String>& GetKeywords() const { return m_keywords; }
		int GetPassCount() const { return m_passes.Size(); }
		const Pass& GetPass(int index) const { return m_passes[index]; }
        int GetQueue() const { return m_queue; }

	private:
		Shader(const String& name, bool light_add);
		void Load(const String& src, const List<String>& keywords);
		void Compile();

	private:
		static Map<String, Ref<Shader>> m_shaders;
		List<String> m_keywords;
		bool m_light_add;
		Vector<Pass> m_passes;
		int m_queue;
    };
}
