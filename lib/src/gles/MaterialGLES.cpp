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
#include "memory/Memory.h"
#include "graphics/Material.h"
#include "graphics/Texture2D.h"
#include "graphics/UniformBuffer.h"
#include "graphics/Camera.h"

#if VR_GLES

namespace Viry3D
{
	void MaterialGLES::UpdateUniformsBegin(int pass_index)
	{
		auto mat = (Material*) this;
		auto shader = mat->GetShader();
		if (Camera::Current()->GetRenderMode() == CameraRenderMode::ShadowMap)
		{
			shader = Shader::ReplaceToShadowMapShader(shader);
		}

		if (Camera::Current()->GetRenderMode() == CameraRenderMode::ShadowMap)
		{
			if (m_uniform_buffers_shadowmap.Size() < pass_index + 1)
			{
				m_uniform_buffers_shadowmap.Resize(pass_index + 1);
			}

			if (!m_uniform_buffers_shadowmap[pass_index])
			{
				m_uniform_buffers_shadowmap[pass_index] = shader->CreateUniformBuffer(pass_index);
			}
		}
		else
		{
			if (m_uniform_buffers.Size() < pass_index + 1)
			{
				m_uniform_buffers.Resize(pass_index + 1);
			}

			if (!m_uniform_buffers[pass_index])
			{
				m_uniform_buffers[pass_index] = shader->CreateUniformBuffer(pass_index);
			}
		}
	}

	void* MaterialGLES::SetUniformBegin(int pass_index)
	{
		LogGLError();

		void* mapped = NULL;

		Ref<UniformBuffer> uniform_buffer;
		if (Camera::Current()->GetRenderMode() == CameraRenderMode::ShadowMap)
		{
			uniform_buffer = m_uniform_buffers_shadowmap[pass_index];
		}
		else
		{
			uniform_buffer = m_uniform_buffers[pass_index];
		}
		
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

		Ref<UniformBuffer> uniform_buffer;
		if (Camera::Current()->GetRenderMode() == CameraRenderMode::ShadowMap)
		{
			uniform_buffer = m_uniform_buffers_shadowmap[pass_index];
		}
		else
		{
			uniform_buffer = m_uniform_buffers[pass_index];
		}

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

	void MaterialGLES::SetUniform(int pass_index, void* uniform_buffer, const String& name, void* data, int size)
	{
		auto buffer = (char*) uniform_buffer;
		auto mat = (Material*) this;
		auto shader = mat->GetShader();
		if (Camera::Current()->GetRenderMode() == CameraRenderMode::ShadowMap)
		{
			shader = Shader::ReplaceToShadowMapShader(shader);
		}

		const auto& uniform_buffer_infos = shader->GetUniformBufferInfos(pass_index);

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

	void MaterialGLES::Apply(int pass_index)
	{
		LogGLError();

		auto mat = (Material*) this;
		auto shader = mat->GetShader();
		if (Camera::Current()->GetRenderMode() == CameraRenderMode::ShadowMap)
		{
			shader = Shader::ReplaceToShadowMapShader(shader);
		}

		auto& sampler_infos = shader->GetSamplerInfos(pass_index);
		auto& sampler_locations = shader->GetSamplerLocations(pass_index);
		auto& textures = mat->GetTextures();

		for (int i = 0; i < sampler_locations.Size(); i++)
		{
			auto location = sampler_locations[i];

			glActiveTexture(GL_TEXTURE0 + i + 1);

			const Ref<Texture>* tex;
			if (textures.TryGet(sampler_infos[i]->name, &tex))
			{
				auto texture = (*tex)->GetTexture();
				if (sampler_infos[i]->type == "2D")
				{
					glBindTexture(GL_TEXTURE_2D, texture);
				}
				else if (sampler_infos[i]->type == "Cube")
				{
					glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
				}
				else
				{
					Log("invalid sampler type!");
				}
			}
			else
			{
				if (sampler_infos[i]->type == "2D")
				{
					auto default_texture = Shader::GetDefaultTexture(sampler_infos[i]->default_tex)->GetTexture();
					glBindTexture(GL_TEXTURE_2D, default_texture);
				}
			}

			glUniform1i(location, i + 1);
		}

		Ref<UniformBuffer> uniform_buffer;
		if (Camera::Current()->GetRenderMode() == CameraRenderMode::ShadowMap)
		{
			uniform_buffer = m_uniform_buffers_shadowmap[pass_index];
		}
		else
		{
			uniform_buffer = m_uniform_buffers[pass_index];
		}

		auto& uniform_buffer_infos = shader->GetUniformBufferInfos(pass_index);
		for (auto i : uniform_buffer_infos)
		{
			glBindBufferRange(GL_UNIFORM_BUFFER, i->binding, uniform_buffer->GetBuffer(), i->offset, i->size);
		}

		LogGLError();
	}
}

#endif
