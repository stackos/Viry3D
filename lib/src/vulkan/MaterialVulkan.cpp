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
#include "memory/Memory.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Shader.h"
#include "graphics/Texture2D.h"
#include "graphics/LightmapSettings.h"
#include "graphics/RenderPass.h"

#if VR_VULKAN

namespace Viry3D
{
	MaterialVulkan::MaterialVulkan()
	{
	}

	MaterialVulkan::~MaterialVulkan()
	{
	}

	const Vector<VkDescriptorSet>& MaterialVulkan::GetDescriptorSet(int pass_index)
	{
		return m_descriptor_sets[pass_index];
	}

	void* MaterialVulkan::SetUniformBegin(int pass_index)
	{
		auto display = (DisplayVulkan*) Graphics::GetDisplay();
		auto device = display->GetDevice();

		if (m_uniform_buffers[pass_index])
		{
			void* mapped;
			VkResult err = vkMapMemory(device, m_uniform_buffers[pass_index]->GetMemory(), 0, m_uniform_buffers[pass_index]->GetSize(), 0, &mapped);
			assert(!err);

			return mapped;
		}

		return NULL;
	}

	void MaterialVulkan::SetUniform(int pass_index, void* uniform_buffer, const String& name, void* data, int size)
	{
		auto buffer = (char*) uniform_buffer;
		auto mat = (Material*) this;
		auto& shader = mat->GetShader();
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

		if (m_uniform_buffers[pass_index])
		{
			vkUnmapMemory(device, m_uniform_buffers[pass_index]->GetMemory());
		}
	}

	void MaterialVulkan::SetUniformTexture(int pass_index, const String& name, const Texture* texture)
	{
		auto mat = (Material*) this;
		auto& shader = mat->GetShader();
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
		auto& shader = mat->GetShader();
		auto& writes = shader->GetDescriptorSetWriteInfo(pass_index);
		auto& uniform_xmls = shader->GetUniformXmls(pass_index);

		if (m_descriptor_sets.Size() < pass_index + 1)
		{
			m_descriptor_sets.Resize(pass_index + 1);
			m_uniform_buffers.Resize(pass_index + 1);
		}

		if (m_descriptor_sets[pass_index].Empty())
		{
			m_descriptor_sets[pass_index] = shader->CreateDescriptorSet(pass_index);
			m_uniform_buffers[pass_index] = shader->CreateUniformBuffer(pass_index);
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

	bool MaterialVulkan::CheckWritesDirty(int pass_index) const
	{
		auto mat = (Material*) this;
		auto& shader = mat->GetShader();
		auto& writes = shader->GetDescriptorSetWriteInfo(pass_index);

		bool write_dirty = false;
		if (m_writes_old.Size() != writes.Size())
		{
			write_dirty = true;
		}
		else
		{
			for (int i = 0; i < writes.Size(); i++)
			{
				if (m_writes_old[i].set.sType != writes[i].sType ||
					m_writes_old[i].set.pNext != writes[i].pNext ||
					m_writes_old[i].set.dstBinding != writes[i].dstBinding ||
					m_writes_old[i].set.dstArrayElement != writes[i].dstArrayElement ||
					m_writes_old[i].set.descriptorCount != writes[i].descriptorCount ||
					m_writes_old[i].set.descriptorType != writes[i].descriptorType ||
					m_writes_old[i].set.pImageInfo != writes[i].pImageInfo ||
					m_writes_old[i].set.pBufferInfo != writes[i].pBufferInfo ||
					m_writes_old[i].set.pTexelBufferView != writes[i].pTexelBufferView)
				{
					write_dirty = true;
					break;
				}
				else
				{
					if (writes[i].pImageInfo != NULL)
					{
						if (m_writes_old[i].image.sampler != writes[i].pImageInfo->sampler ||
							m_writes_old[i].image.imageView != writes[i].pImageInfo->imageView ||
							m_writes_old[i].image.imageLayout != writes[i].pImageInfo->imageLayout)
						{
							write_dirty = true;
							break;
						}
					}

					if (writes[i].pBufferInfo != NULL)
					{
						if (m_writes_old[i].buffer.buffer != writes[i].pBufferInfo->buffer ||
							m_writes_old[i].buffer.offset != writes[i].pBufferInfo->offset ||
							m_writes_old[i].buffer.range != writes[i].pBufferInfo->range)
						{
							write_dirty = true;
							break;
						}
					}
				}
			}
		}

		return write_dirty;
	}

	void MaterialVulkan::UpdateUniformsEnd(int pass_index)
	{
		auto mat = (Material*) this;
		auto& shader = mat->GetShader();
		auto& writes = shader->GetDescriptorSetWriteInfo(pass_index);
		auto display = (DisplayVulkan*) Graphics::GetDisplay();
		auto device = display->GetDevice();

		for (int i = 0; i < writes.Size(); i++)
		{
			auto& write = writes[i];

			if (write.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			{
				void* p = (void*) write.pBufferInfo;
				VkDescriptorBufferInfo* uniform_info = (VkDescriptorBufferInfo*) p;
				uniform_info->buffer = m_uniform_buffers[pass_index]->GetBuffer();
			}
		}

		if (CheckWritesDirty(pass_index))
		{
			int lightmap_count = LightmapSettings::GetLightmapCount();
			auto& descriptor_sets = m_descriptor_sets[pass_index];
			for (int i = 0; i < descriptor_sets.Size(); i++)
			{
				if (i < lightmap_count)
				{
					SetUniformTexture(pass_index, "_Lightmap", LightmapSettings::GetLightmap(i));
				}

				for (int j = 0; j < writes.Size(); j++)
				{
					auto& write = writes[j];
					write.dstSet = descriptor_sets[i];
				}

				vkUpdateDescriptorSets(device, writes.Size(), &writes[0], 0, NULL);
			}

			m_writes_old.Resize(writes.Size());
			for (int i = 0; i < m_writes_old.Size(); i++)
			{
				m_writes_old[i].set = writes[i];
				if (writes[i].pImageInfo != NULL)
				{
					m_writes_old[i].image = *writes[i].pImageInfo;
				}
				if (writes[i].pBufferInfo != NULL)
				{
					m_writes_old[i].buffer = *writes[i].pBufferInfo;
				}
			}

			RenderPass::GetRenderPassBinding()->SetCommandDirty();
		}
	}
}

#endif
