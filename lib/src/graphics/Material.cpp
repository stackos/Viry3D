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

#include "Material.h"
#include "Camera.h"

namespace Viry3D
{
	static const String MAIN_TEX_NAME = "_MainTex";
	static const String MAIN_COLOR_NAME = "_Color";

	Ref<Material> Material::Create(const String& shader_name)
	{
		Ref<Material> mat;
		
		mat = Ref<Material>(new Material());
		mat->SetName(shader_name);

		auto shader = Shader::Find(shader_name);
		if (shader)
		{
			mat->m_shader = shader;
		}
		else
		{
			mat->m_shader = Shader::Find("Error");
		}

		return mat;
	}

	void Material::DeepCopy(const Ref<Object>& source)
	{
		Object::DeepCopy(source);

		auto src = RefCast<Material>(source);
		this->m_matrices = src->m_matrices;
		this->m_vectors = src->m_vectors;
		this->m_vector_arrays = src->m_vector_arrays;
		this->m_textures = src->m_textures;
		this->m_colors = src->m_colors;
	}

	Material::Material()
	{
		this->SetMainColor(Color(1, 1, 1, 1));
	}

	void Material::SetShader(const Ref<Shader>& shader)
	{
		if (shader)
		{
			m_shader = shader;
			this->OnShaderChanged();
		}
	}

	void Material::SetMatrix(const String& name, const Matrix4x4& v)
	{
		if (!m_matrices.Contains(name))
		{
			m_matrices.Add(name, v);
		}
		else
		{
			m_matrices[name] = v;
		}
	}

	const Matrix4x4& Material::GetMatrix(const String& name) const
	{
		return m_matrices[name];
	}

	void Material::SetVector(const String& name, const Vector4& v)
	{
		if (!m_vectors.Contains(name))
		{
			m_vectors.Add(name, v);
		}
		else
		{
			m_vectors[name] = v;
		}
	}

	bool Material::HasVector(const String& name) const
	{
		return m_vectors.Contains(name);
	}

	const Vector4& Material::GetVector(const String& name) const
	{
		return m_vectors[name];
	}

	void Material::SetMainColor(const Color& v)
	{
		this->SetColor(MAIN_COLOR_NAME, v);
	}

	const Color& Material::GetMainColor() const
	{
		return this->GetColor(MAIN_COLOR_NAME);
	}

	void Material::SetColor(const String& name, const Color& v)
	{
		if (!m_colors.Contains(name))
		{
			m_colors.Add(name, v);
		}
		else
		{
			m_colors[name] = v;
		}
	}

	const Color& Material::GetColor(const String& name) const
	{
		return m_colors[name];
	}

	void Material::SetVectorArray(const String& name, const Vector<Vector4>& v)
	{
		if (!m_vector_arrays.Contains(name))
		{
			m_vector_arrays.Add(name, v);
		}
		else
		{
			m_vector_arrays[name] = v;
		}
	}

	const Vector<Vector4>& Material::GetVectorArray(const String& name) const
	{
		return m_vector_arrays[name];
	}

	void Material::SetMainTexture(const Ref<Texture>& v)
	{
		this->SetTexture(MAIN_TEX_NAME, v);
	}

	bool Material::HasMainTexture() const
	{
		return m_textures.Contains(MAIN_TEX_NAME);
	}

	const Ref<Texture>& Material::GetMainTexture() const
	{
		return m_textures[MAIN_TEX_NAME];
	}

	void Material::SetTexture(const String& name, const Ref<Texture>& v)
	{
		if (!m_textures.Contains(name))
		{
			m_textures.Add(name, v);
		}
		else
		{
			m_textures[name] = v;
		}
	}

	void Material::SetZBufferParams(const Ref<Camera>& cam)
	{
		float cam_far = cam->GetClipFar();
		float cam_near = cam->GetClipNear();

#if VR_VULKAN
		float zx = (1.0f - cam_far / cam_near) / 2;
		float zy = (1.0f + cam_far / cam_near) / 2;
#else
		float zx = (1.0f - cam_far / cam_near);
		float zy = (cam_far / cam_near);
#endif

		SetVector("_ZBufferParams", Vector4(zx, zy, zx / cam_far, zy / cam_near));
	}

	void Material::SetProjectionParams(const Ref<Camera>& cam)
	{
		float cam_far = cam->GetClipFar();
		float cam_near = cam->GetClipNear();

		// x = 1 or -1 (-1 if projection is flipped)
		// y = near plane
		// z = far plane
		// w = 1/far plane
		SetVector("_ProjectionParams", Vector4(1, cam_near, cam_far, 1 / cam_far));
	}

	void Material::SetMainTexTexelSize(const Ref<Texture>& tex)
	{
		SetVector("_MainTex_TexelSize", Vector4(1.0f / tex->GetWidth(), 1.0f / tex->GetHeight(), (float) tex->GetWidth(), (float) tex->GetHeight()));
	}

	void Material::UpdateUniforms(int pass_index)
	{
		this->UpdateUniformsBegin(pass_index);

		auto buffer = this->SetUniformBegin(pass_index);
		for (auto& i : m_matrices)
		{
			this->SetUniform(pass_index, buffer, i.first, (void*) &i.second, sizeof(Matrix4x4));
		}
		for (auto& i : m_colors)
		{
			this->SetUniform(pass_index, buffer, i.first, (void*) &i.second, sizeof(Color));
		}
		for (auto& i : m_vectors)
		{
			this->SetUniform(pass_index, buffer, i.first, (void*) &i.second, sizeof(Vector4));
		}
		for (auto& i : m_vector_arrays)
		{
			this->SetUniform(pass_index, buffer, i.first, (void*) &i.second[0], i.second.SizeInBytes());
		}
		this->SetUniformEnd(pass_index);

		for (auto& i : m_textures)
		{
			this->SetUniformTexture(pass_index, i.first, i.second.get());
		}

		this->UpdateUniformsEnd(pass_index);
	}
}
