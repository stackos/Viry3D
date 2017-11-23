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

#include "MaterialVulkan.h"
#include "DisplayVulkan.h"
#include "DescriptorSetVulkan.h"
#include "memory/Memory.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Shader.h"
#include "graphics/Texture2D.h"
#include "graphics/LightmapSettings.h"
#include "graphics/RenderPass.h"
#include "graphics/Camera.h"

#if VR_VULKAN

namespace Viry3D
{
	void MaterialVulkan::OnShaderChanged()
	{
		m_descriptor_sets.Clear();
		m_uniform_buffers.Clear();
	}

	const Ref<DescriptorSet>& MaterialVulkan::GetDescriptorSet(int pass_index)
	{
		if (Camera::Current()->GetRenderMode() == CameraRenderMode::ShadowMap)
		{
			return m_descriptor_sets_shadowmap[pass_index];
		}
		
		return m_descriptor_sets[pass_index];
	}

	void* MaterialVulkan::SetUniformBegin(int pass_index)
	{
		auto display = (DisplayVulkan*) Graphics::GetDisplay();
		auto device = display->GetDevice();

		if (Camera::Current()->GetRenderMode() == CameraRenderMode::ShadowMap)
		{
			if (m_uniform_buffers_shadowmap[pass_index])
			{
				void* mapped;
				VkResult err = vkMapMemory(device, m_uniform_buffers_shadowmap[pass_index]->GetMemory(), 0, m_uniform_buffers_shadowmap[pass_index]->GetSize(), 0, &mapped);
				assert(!err);

				return mapped;
			}
		}
		else
		{
			if (m_uniform_buffers[pass_index])
			{
				void* mapped;
				VkResult err = vkMapMemory(device, m_uniform_buffers[pass_index]->GetMemory(), 0, m_uniform_buffers[pass_index]->GetSize(), 0, &mapped);
				assert(!err);

				return mapped;
			}
		}

		return NULL;
	}

	void MaterialVulkan::SetUniform(int pass_index, void* uniform_buffer, const String& name, void* data, int size)
	{
		auto buffer = (char*) uniform_buffer;
		auto mat = (Material*) this;
		auto shader = mat->GetShader();
		if (Camera::Current()->GetRenderMode() == CameraRenderMode::ShadowMap)
		{
			shader = Shader::ReplaceToShadowMapShader(shader);
		}

		auto& writes = shader->GetDescriptorSetWriteInfo(pass_index);
		auto& uniform_xmls = shader->GetUniformXmls(pass_index);

		for (int i = 0; i < writes.Size(); i++)
		{
			auto& write = writes[i];

			if (write.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			{
				auto uniform_info = write.pBufferInfo;

				auto offset = uniform_info->offset;
				auto& uniform_buffer_info = *(XMLUniformBuffer*) uniform_xmls[i];

				for (auto& j : uniform_buffer_info.uniforms)
				{
					if (j.name == name)
					{
						assert(j.size >= size);

						Memory::Copy(&buffer[offset + j.offset], data, size);
						break;
					}
				}
			}
		}
	}

	void MaterialVulkan::SetUniformEnd(int pass_index)
	{
		auto display = (DisplayVulkan*) Graphics::GetDisplay();
		auto device = display->GetDevice();

		if (Camera::Current()->GetRenderMode() == CameraRenderMode::ShadowMap)
		{
			if (m_uniform_buffers_shadowmap[pass_index])
			{
				vkUnmapMemory(device, m_uniform_buffers_shadowmap[pass_index]->GetMemory());
			}
		}
		else
		{
			if (m_uniform_buffers[pass_index])
			{
				vkUnmapMemory(device, m_uniform_buffers[pass_index]->GetMemory());
			}
		}
	}

	void MaterialVulkan::SetUniformTexture(int pass_index, const String& name, const Texture* texture)
	{
		auto mat = (Material*) this;
		auto shader = mat->GetShader();
		if (Camera::Current()->GetRenderMode() == CameraRenderMode::ShadowMap)
		{
			shader = Shader::ReplaceToShadowMapShader(shader);
		}

		auto& writes = shader->GetDescriptorSetWriteInfo(pass_index);
		auto& uniform_xmls = shader->GetUniformXmls(pass_index);

		for (int i = 0; i < writes.Size(); i++)
		{
			auto& write = writes[i];

			if (write.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				auto& sampler_xml_info = *(XMLSampler*) uniform_xmls[i];

				if (sampler_xml_info.name == name)
				{
					auto tex = (TextureVulkan*) texture;
					if (tex)
					{
						void* p = (void*) write.pImageInfo;
						VkDescriptorImageInfo* sampler_info = (VkDescriptorImageInfo*) p;

						sampler_info->sampler = tex->GetSampler();
						sampler_info->imageView = tex->GetImageView();
					}

					break;
				}
			}
		}
	}

	void MaterialVulkan::UpdateUniformsBegin(int pass_index)
	{
		auto mat = (Material*) this;
		auto shader = mat->GetShader();
		if (Camera::Current()->GetRenderMode() == CameraRenderMode::ShadowMap)
		{
			shader = Shader::ReplaceToShadowMapShader(shader);
		}

		auto& writes = shader->GetDescriptorSetWriteInfo(pass_index);
		auto& uniform_xmls = shader->GetUniformXmls(pass_index);

		if (Camera::Current()->GetRenderMode() == CameraRenderMode::ShadowMap)
		{
			if (m_descriptor_sets_shadowmap.Size() < pass_index + 1)
			{
				m_descriptor_sets_shadowmap.Resize(pass_index + 1);
				m_uniform_buffers_shadowmap.Resize(pass_index + 1);
			}

			if (!m_descriptor_sets_shadowmap[pass_index])
			{
				auto ds = RefMake<DescriptorSetVulkan>();
				ds->set = shader->CreateDescriptorSet(pass_index);
				m_descriptor_sets_shadowmap[pass_index] = ds;

				m_uniform_buffers_shadowmap[pass_index] = shader->CreateUniformBuffer(pass_index);
			}
		}
		else
		{
			if (m_descriptor_sets.Size() < pass_index + 1)
			{
				m_descriptor_sets.Resize(pass_index + 1);
				m_uniform_buffers.Resize(pass_index + 1);
			}

			if (!m_descriptor_sets[pass_index])
			{
				auto ds = RefMake<DescriptorSetVulkan>();
				ds->set = shader->CreateDescriptorSet(pass_index);
				m_descriptor_sets[pass_index] = ds;

				m_uniform_buffers[pass_index] = shader->CreateUniformBuffer(pass_index);
			}
		}

		for (int i = 0; i < writes.Size(); i++)
		{
			auto& write = writes[i];

			if (write.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				auto& sampler_xml_info = *(XMLSampler*) uniform_xmls[i];

				void* p = (void*) write.pImageInfo;
				VkDescriptorImageInfo* sampler_info = (VkDescriptorImageInfo*) p;

				auto& default_tex = Shader::GetDefaultTexture(sampler_xml_info.default_tex);
				sampler_info->sampler = default_tex->GetSampler();
				sampler_info->imageView = default_tex->GetImageView();
			}
		}
	}

	void MaterialVulkan::UpdateUniformsEnd(int pass_index)
	{
		auto mat = (Material*) this;
		auto shader = mat->GetShader();
		if (Camera::Current()->GetRenderMode() == CameraRenderMode::ShadowMap)
		{
			shader = Shader::ReplaceToShadowMapShader(shader);
		}

		auto& writes = shader->GetDescriptorSetWriteInfo(pass_index);
		auto display = (DisplayVulkan*) Graphics::GetDisplay();
		auto device = display->GetDevice();

		for (int i = 0; i < writes.Size(); i++)
		{
			auto& write = writes[i];

			if (Camera::Current()->GetRenderMode() == CameraRenderMode::ShadowMap)
			{
				if (write.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
				{
					void* p = (void*) write.pBufferInfo;
					VkDescriptorBufferInfo* uniform_info = (VkDescriptorBufferInfo*) p;
					uniform_info->buffer = m_uniform_buffers_shadowmap[pass_index]->GetBuffer();
				}

				write.dstSet = RefCast<DescriptorSetVulkan>(m_descriptor_sets_shadowmap[pass_index])->set;
			}
			else
			{
				if (write.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
				{
					void* p = (void*) write.pBufferInfo;
					VkDescriptorBufferInfo* uniform_info = (VkDescriptorBufferInfo*) p;
					uniform_info->buffer = m_uniform_buffers[pass_index]->GetBuffer();
				}

				write.dstSet = RefCast<DescriptorSetVulkan>(m_descriptor_sets[pass_index])->set;
			}
		}

		vkUpdateDescriptorSets(device, writes.Size(), &writes[0], 0, NULL);
	}
}

#endif
