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

#include "MaterialGLES.h"
#include "gles_include.h"
#include "Debug.h"
#include "graphics/Material.h"
#include "graphics/Texture2D.h"
#include "graphics/UniformBuffer.h"
#include "graphics/LightmapSettings.h"
#include "memory/Memory.h"

#if VR_GLES

namespace Viry3D
{
	static const String LIGHTMAP_NAME = "_Lightmap";

	void MaterialGLES::UpdateUniformsBegin(int pass_index)
	{
		auto mat = (Material*) this;
		auto shader = mat->GetShader();

		if (m_uniform_buffers.Size() < pass_index + 1)
		{
			m_uniform_buffers.Resize(pass_index + 1);
		}

		if (!m_uniform_buffers[pass_index])
		{
			m_uniform_buffers[pass_index] = shader->CreateUniformBuffer(pass_index);
		}
	}

	void* MaterialGLES::SetUniformBegin(int pass_index)
	{
		LogGLError();

		void* mapped = NULL;

		auto uniform_buffer = m_uniform_buffers[pass_index];
		if (uniform_buffer)
		{
			glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer->GetBuffer());
			//mapped = glMapBufferRange(GL_UNIFORM_BUFFER, 0, uniform_buffer->GetSize(), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

			mapped = uniform_buffer->GetLocalBuffer()->Bytes();
		}

		LogGLError();

		return mapped;
	}

	void MaterialGLES::SetUniformEnd(int pass_index)
	{
		LogGLError();

		auto uniform_buffer = m_uniform_buffers[pass_index];
		if (uniform_buffer)
		{
			/*
			auto unmap_result = glUnmapBuffer(GL_UNIFORM_BUFFER);
			if(unmap_result == GL_FALSE)
			{
			Log("glUnmapBuffer failed");
			}
			*/

			glBufferSubData(GL_UNIFORM_BUFFER, 0, uniform_buffer->GetSize(), uniform_buffer->GetLocalBuffer()->Bytes());

			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		LogGLError();
	}

	void MaterialGLES::SetUniform(int pass_index, void* uniform_buffer, String name, void* data, int size)
	{
		auto buffer = (char*) uniform_buffer;
		auto mat = (Material*) this;
		auto shader = mat->GetShader();
		auto uniform_buffer_infos = shader->GetUniformBufferInfos(pass_index);

		for (auto i : uniform_buffer_infos)
		{
			for (auto& j : i->uniforms)
			{
				if (j.name == name)
				{
					assert(j.size >= size);

					Memory::Copy(&buffer[i->offset + j.offset], data, size);
					break;
				}
			}
		}
	}

	void MaterialGLES::ApplyLightmap(int pass_index, int lightmap_index)
	{
		auto mat = (Material*) this;
		auto& shader = mat->GetShader();
		auto& sampler_infos = shader->GetSamplerInfos(pass_index);
		auto& sampler_locations = shader->GetSamplerLocations(pass_index);

		for (int i = 0; i < sampler_locations.Size(); i++)
		{
			if (sampler_infos[i]->name == LIGHTMAP_NAME)
			{
				auto location = sampler_locations[i];

				glActiveTexture(GL_TEXTURE0 + i);

				auto texture = LightmapSettings::GetLightmap(lightmap_index)->GetTexture();
				glBindTexture(GL_TEXTURE_2D, texture);

				glUniform1i(location, i);
				break;
			}
		}
	}

	void MaterialGLES::Apply(int pass_index)
	{
		LogGLError();

		auto mat = (Material*) this;
		auto& shader = mat->GetShader();
		auto& sampler_infos = shader->GetSamplerInfos(pass_index);
		auto& sampler_locations = shader->GetSamplerLocations(pass_index);
		auto& textures = mat->GetTextures();

		for (int i = 0; i < sampler_locations.Size(); i++)
		{
			auto location = sampler_locations[i];

			glActiveTexture(GL_TEXTURE0 + i);

			auto& name = sampler_infos[i]->name;
			const Ref<Texture>* tex;
			if (textures.TryGet(name, &tex))
			{
				auto texture = (*tex)->GetTexture();
				glBindTexture(GL_TEXTURE_2D, texture);
			}
			else
			{
				auto default_texture = Shader::GetDefaultTexture(sampler_infos[i]->default_tex)->GetTexture();
				glBindTexture(GL_TEXTURE_2D, default_texture);
			}

			glUniform1i(location, i);
		}

		auto& uniform_buffer_infos = shader->GetUniformBufferInfos(pass_index);
		auto& uniform_buffer = m_uniform_buffers[pass_index];

		for (auto i : uniform_buffer_infos)
		{
			glBindBufferRange(GL_UNIFORM_BUFFER, i->binding, uniform_buffer->GetBuffer(), i->offset, i->size);
		}

		LogGLError();
	}
}

#endif
