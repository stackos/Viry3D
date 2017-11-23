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

#include "Object.h"
#include "Texture.h"
#include "Shader.h"
#include "Color.h"
#include "math/Matrix4x4.h"

#if VR_VULKAN
#include "vulkan/MaterialVulkan.h"
#elif VR_GLES
#include "gles/MaterialGLES.h"
#endif

namespace Viry3D
{
	class Camera;

#if VR_VULKAN
	class Material: public MaterialVulkan
#elif VR_GLES
	class Material: public MaterialGLES
#endif
	{
	public:
		static Ref<Material> Create(const String& shader_name);

		virtual void DeepCopy(const Ref<Object>& source);

		const Ref<Shader>& GetShader() const { return m_shader; }
		void SetShader(const Ref<Shader>& shader);

		void SetMatrix(const String& name, const Matrix4x4& v);
		const Matrix4x4& GetMatrix(const String& name) const;
		void SetVector(const String& name, const Vector4& v);
		bool HasVector(const String& name) const;
		const Vector4& GetVector(const String& name) const;
		void SetMainColor(const Color& v);
		const Color& GetMainColor() const;
		void SetColor(const String& name, const Color& v);
		const Color& GetColor(const String& name) const;
		void SetVectorArray(const String& name, const Vector<Vector4>& v);
		const Vector<Vector4>& GetVectorArray(const String& name) const;
		void SetMainTexture(const Ref<Texture>& v);
		bool HasMainTexture() const;
		const Ref<Texture>& GetMainTexture() const;
		void SetTexture(const String& name, const Ref<Texture>& v);
		const Map<String, Ref<Texture>>& GetTextures() const { return m_textures; }
		void SetMainTexTexelSize(const Ref<Texture>& tex);
		void SetZBufferParams(const Ref<Camera>& cam);
		void SetProjectionParams(const Ref<Camera>& cam);

		void UpdateUniforms(int pass_index);

	private:
		Material();

		Ref<Shader> m_shader;
		Map<String, Matrix4x4> m_matrices;
		Map<String, Vector4> m_vectors;
		Map<String, Vector<Vector4>> m_vector_arrays;
		Map<String, Ref<Texture>> m_textures;
		Map<String, Color> m_colors;
	};
}
