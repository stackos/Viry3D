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

#if VR_VULKAN

#include "ShaderVulkan.h"
#include "DisplayVulkan.h"
#include "MaterialVulkan.h"
#include "Application.h"
#include "vulkan_shader_compiler.h"
#include "graphics/Graphics.h"
#include "graphics/Shader.h"
#include "graphics/Camera.h"
#include "graphics/RenderPass.h"
#include "graphics/Material.h"
#include "graphics/LightmapSettings.h"
#include "io/File.h"
#include "memory/Memory.h"

extern "C"
{
#include "crypto/md5/md5.h"
}

namespace Viry3D
{
	static void compile_with_cache(Vector<unsigned int>& spirv, const String& src, const VkShaderStageFlagBits shader_type)
	{
		unsigned char hash_bytes[16];
		MD5_CTX md5_context;
		MD5_Init(&md5_context);
		MD5_Update(&md5_context, (void*) src.CString(), src.Size());
		MD5_Final(hash_bytes, &md5_context);
		String md5_str;
		for (int i = 0; i < sizeof(hash_bytes); i++)
		{
			md5_str += String::Format("%02x", hash_bytes[i]);
		}

		String cache_path = Application::SavePath() + "/" + md5_str + ".cache";
		if (File::Exist(cache_path))
		{
			auto buffer = File::ReadAllBytes(cache_path);
			spirv.Resize(buffer.Size() / 4);
			Memory::Copy(&spirv[0], buffer.Bytes(), buffer.Size());
		}
		else
		{
			String error;
			bool success = glsl_to_spv(shader_type, src.CString(), spirv, error);
			assert(success);

			ByteBuffer buffer(spirv.SizeInBytes());
			Memory::Copy(buffer.Bytes(), &spirv[0], buffer.Size());
			File::WriteAllBytes(cache_path, buffer);
		}
	}

	static VkShaderModule create_shader_module(VkDevice device, void* code, int size)
	{
		VkShaderModuleCreateInfo create_info;
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.pNext = NULL;
		create_info.codeSize = size;
		create_info.pCode = (const uint32_t *) code;
		create_info.flags = 0;

		VkShaderModule module;
		VkResult err = vkCreateShaderModule(device, &create_info, NULL, &module);
		assert(!err);

		return module;
	}

	static void create_descriptor_set_info(const XMLPass& pass, const XMLShader& xml, ShaderPass& shader_pass)
	{
		auto display = (DisplayVulkan*) Graphics::GetDisplay();
		auto device = display->GetDevice();

		//	create descriptor set layout
		Vector<VkDescriptorSetLayoutBinding>& binds = shader_pass.binds;

		for (const auto& i : xml.vss)
		{
			if (pass.vs == i.name)
			{
				if (i.uniform_buffer.binding >= 0 && i.uniform_buffer.size > 0)
				{
					VkDescriptorSetLayoutBinding binding;
					binding.binding = i.uniform_buffer.binding;
					binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					binding.descriptorCount = 1;
					binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
					binding.pImmutableSamplers = NULL;

					binds.Add(binding);

					shader_pass.uniform_xmls.Add(&i.uniform_buffer);
				}
				break;
			}
		}

		for (const auto& i : xml.pss)
		{
			if (pass.ps == i.name)
			{
				if (i.uniform_buffer.binding >= 0 && i.uniform_buffer.size > 0)
				{
					VkDescriptorSetLayoutBinding binding;
					binding.binding = i.uniform_buffer.binding;
					binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					binding.descriptorCount = 1;
					binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
					binding.pImmutableSamplers = NULL;

					binds.Add(binding);

					shader_pass.uniform_xmls.Add(&i.uniform_buffer);
				}

				for (const auto& j : i.samplers)
				{
					VkDescriptorSetLayoutBinding binding;
					binding.binding = j.binding;
					binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					binding.descriptorCount = 1;
					binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
					binding.pImmutableSamplers = NULL;

					binds.Add(binding);

					shader_pass.uniform_xmls.Add(&j);
				}
				break;
			}
		}

		VkDescriptorSetLayoutCreateInfo layout_info = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			NULL,
			0,
			(uint32_t) binds.Size(),
			binds.Empty() ? NULL : &binds[0],
		};

		VkResult err = vkCreateDescriptorSetLayout(device, &layout_info, NULL, &shader_pass.descriptor_layout);
		assert(!err);

		//	create descriptor pool
		Vector<VkDescriptorPoolSize> type_counts;

		for (const auto& i : binds)
		{
			VkDescriptorPoolSize type_count;
			type_count.type = i.descriptorType;
			type_count.descriptorCount = DESCRIPTOR_POOL_SIZE_MAX;

			type_counts.Add(type_count);
		}

		if (!type_counts.Empty())
		{
			VkDescriptorPoolCreateInfo pool_info = {
				VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
				NULL,
				0,
				DESCRIPTOR_POOL_SIZE_MAX,
				(uint32_t) type_counts.Size(),
				&type_counts[0],
			};

			err = vkCreateDescriptorPool(device, &pool_info, NULL, &shader_pass.descriptor_pool);
			assert(!err);
		}
		else
		{
			shader_pass.descriptor_pool = NULL;
		}

		//	create uniform info and sampler info
		{
			int buffer_size = 0;
			for (int i = 0; i < binds.Size(); i++)
			{
				if (binds[i].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
				{
					auto& uniform_buffer_info = *(XMLUniformBuffer*) shader_pass.uniform_xmls[i];

					int offset = buffer_size;
					int alignment = display->GetMinUniformBufferOffsetAlignment();

					if (offset % alignment != 0)
					{
						offset += alignment - offset % alignment;
						buffer_size = offset;
					}

					VkDescriptorBufferInfo info;
					info.buffer = NULL;
					info.offset = offset;
					info.range = uniform_buffer_info.size;
					shader_pass.uniform_infos.Add(info);

					buffer_size += uniform_buffer_info.size;
				}
				else if (binds[i].descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
				{
					VkDescriptorImageInfo sampler_info;
					sampler_info.sampler = NULL;
					sampler_info.imageView = NULL;
					sampler_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

					shader_pass.sampler_infos.Add(sampler_info);
				}
			}
		}

		int uniform_info_index = 0;
		int sampler_info_index = 0;
		for (const auto& i : binds)
		{
			VkWriteDescriptorSet write;
			Memory::Zero(&write, sizeof(write));
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstBinding = i.binding;
			write.descriptorCount = 1;
			write.descriptorType = i.descriptorType;
			write.dstSet = NULL;

			if (i.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			{
				write.pBufferInfo = &shader_pass.uniform_infos[uniform_info_index];
				uniform_info_index++;
			}
			else if (i.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				write.pImageInfo = &shader_pass.sampler_infos[sampler_info_index];
				sampler_info_index++;
			}

			shader_pass.uniform_writes.Add(write);
		}
	}

	Vector<VkDescriptorSet> ShaderVulkan::CreateDescriptorSet(int index)
	{
		auto display = (DisplayVulkan*) Graphics::GetDisplay();
		auto device = display->GetDevice();
		auto& pass = m_passes[index];
		VkResult err;

		Vector<VkDescriptorSet> descriptor_sets(LIGHT_MAP_COUNT_MAX);

		for (int i = 0; i < descriptor_sets.Size(); i++)
		{
			VkDescriptorSetAllocateInfo sets_info = {
				VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
				NULL,
				pass.descriptor_pool,
				1,
				&pass.descriptor_layout
			};

			err = vkAllocateDescriptorSets(device, &sets_info, &descriptor_sets[i]);
			assert(!err);
		}

		return descriptor_sets;
	}

	Ref<UniformBuffer> ShaderVulkan::CreateUniformBuffer(int index)
	{
		Ref<UniformBuffer> uniform_buffer;

		auto& pass = m_passes[index];
		Vector<VkDescriptorSetLayoutBinding>& binds = pass.binds;

		int buffer_size = 0;
		auto writes = this->GetDescriptorSetWriteInfo(index);
		for (int i = writes.Size() - 1; i >= 0; i--)
		{
			auto& write = writes[i];
			if (write.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			{
				buffer_size = (int) (write.pBufferInfo->offset + write.pBufferInfo->range);
				break;
			}
		}

		if (buffer_size > 0)
		{
			uniform_buffer = UniformBuffer::Create(buffer_size);
		}

		return uniform_buffer;
	}

	Vector<VkWriteDescriptorSet>& ShaderVulkan::GetDescriptorSetWriteInfo(int index)
	{
		auto& pass = m_passes[index];

		return pass.uniform_writes;
	}

	const Vector<const void*>& ShaderVulkan::GetUniformXmls(int index)
	{
		auto& pass = m_passes[index];

		return pass.uniform_xmls;
	}

	static VkPipelineLayout create_pipeline_layout(VkDescriptorSetLayout& descriptor_layout)
	{
		auto display = (DisplayVulkan*) Graphics::GetDisplay();
		auto device = display->GetDevice();

		const uint32_t push_count = 1;
		VkPushConstantRange pushs[push_count];
		pushs[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushs[0].offset = 0;
		pushs[0].size = sizeof(Matrix4x4) + sizeof(Vector4);

		VkPipelineLayoutCreateInfo create_info;
		Memory::Zero(&create_info, sizeof(create_info));
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		create_info.pNext = NULL;
		create_info.setLayoutCount = 1;
		create_info.pushConstantRangeCount = push_count;
		create_info.pPushConstantRanges = pushs;
		create_info.pSetLayouts = &descriptor_layout;

		VkPipelineLayout layout;
		VkResult err = vkCreatePipelineLayout(device, &create_info, NULL, &layout);
		assert(!err);

		return layout;
	}

	static void prepare_pipeline(
		const XMLPass& pass,
		const XMLShader& xml,
		ShaderPass& shader_pass,
		VkShaderModule vs,
		VkShaderModule ps)
	{
		auto& pipeline_info = shader_pass.pipeline_info;
		VkPipelineDynamicStateCreateInfo& dynamic_state = shader_pass.dynamic_state;
		VkPipelineVertexInputStateCreateInfo& vi = shader_pass.vi;
		VkPipelineInputAssemblyStateCreateInfo& ia = shader_pass.ia;
		VkPipelineRasterizationStateCreateInfo& rs = shader_pass.rs;
		VkPipelineColorBlendStateCreateInfo& cb = shader_pass.cb;
		VkPipelineDepthStencilStateCreateInfo& ds = shader_pass.ds;
		VkPipelineViewportStateCreateInfo& vp = shader_pass.vp;
		VkPipelineMultisampleStateCreateInfo& ms = shader_pass.ms;

		Vector<VkVertexInputAttributeDescription>& vi_attrs = shader_pass.vi_attrs;
		int stride = 0;

		for (auto& i : xml.vss)
		{
			if (pass.vs == i.name)
			{
				stride = i.stride;

				const int values[] = {
					VK_FORMAT_R32G32B32_SFLOAT,
					VK_FORMAT_R32G32B32A32_SFLOAT,
					VK_FORMAT_R32G32_SFLOAT,
					VK_FORMAT_R32G32_SFLOAT,
					VK_FORMAT_R32G32B32_SFLOAT,
					VK_FORMAT_R32G32B32A32_SFLOAT,
					VK_FORMAT_R32G32B32A32_SFLOAT,
					VK_FORMAT_R32G32B32A32_SFLOAT,
				};

				for (auto& j : i.attrs)
				{
					VkVertexInputAttributeDescription attr;
					attr.binding = 0;
					attr.location = j.location;
					attr.offset = j.offset;

					for (int i = 0; i < VertexAttributeType::Count; i++)
					{
						if (j.name == VERTEX_ATTR_TYPES[i])
						{
							attr.format = (VkFormat) values[i];
							break;
						}
					}

					vi_attrs.Add(attr);
				}
				break;
			}
		}

		const XMLRenderState *prs = NULL;
		for (auto& i : xml.rss)
		{
			if (pass.rs == i.name)
			{
				prs = &i;
				break;
			}
		}
		assert(prs != NULL);

		VkVertexInputBindingDescription* vi_bindings = shader_pass.vi_bindings;
		vi_bindings[0].binding = 0;
		vi_bindings[0].stride = stride;
		vi_bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		Memory::Zero(&vi, sizeof(vi));
		vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vi.pNext = NULL;
		vi.vertexBindingDescriptionCount = 1;
		vi.pVertexBindingDescriptions = vi_bindings;
		vi.vertexAttributeDescriptionCount = vi_attrs.Size();
		vi.pVertexAttributeDescriptions = &vi_attrs[0];

		Memory::Zero(shader_pass.dynamic_state_enables, sizeof(shader_pass.dynamic_state_enables));
		VkDynamicState* dynamic_state_enables = shader_pass.dynamic_state_enables;

		Memory::Zero(&dynamic_state, sizeof(dynamic_state));
		dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state.pDynamicStates = dynamic_state_enables;

		Memory::Zero(&vp, sizeof(vp));
		vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		vp.viewportCount = 1;
		dynamic_state_enables[dynamic_state.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
		vp.scissorCount = 1;
		dynamic_state_enables[dynamic_state.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;

		Memory::Zero(&pipeline_info, sizeof(pipeline_info));
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.layout = shader_pass.pipeline_layout;

		Memory::Zero(&ia, sizeof(ia));
		ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		Memory::Zero(&rs, sizeof(rs));
		rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rs.polygonMode = VK_POLYGON_MODE_FILL;
		rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rs.depthClampEnable = VK_FALSE;
		rs.rasterizerDiscardEnable = VK_FALSE;
		rs.depthBiasEnable = VK_FALSE;
		rs.lineWidth = 1.0f;

		rs.depthBiasEnable = prs->Offset.enable;
		if (prs->Offset.enable)
		{
			rs.depthBiasClamp = 0;
			rs.depthBiasSlopeFactor = prs->Offset.factor;
			rs.depthBiasConstantFactor = prs->Offset.units;
		}

		{
			if (prs->Cull == "Back")
			{
				rs.cullMode = VK_CULL_MODE_BACK_BIT;
			}
			else if (prs->Cull == "Front")
			{
				rs.cullMode = VK_CULL_MODE_FRONT_BIT;
			}
			else if (prs->Cull == "Off")
			{
				rs.cullMode = VK_CULL_MODE_NONE;
			}
		}

		Memory::Zero(&cb, sizeof(cb));
		cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

		Memory::Zero(shader_pass.att_state, sizeof(shader_pass.att_state));
		VkPipelineColorBlendAttachmentState* att_state = shader_pass.att_state;
		cb.attachmentCount = 1;
		cb.pAttachments = att_state;

		int color_write = 0;
		{
			if (prs->ColorMask.Contains("R"))
			{
				color_write = color_write | VK_COLOR_COMPONENT_R_BIT;
			}
			if (prs->ColorMask.Contains("G"))
			{
				color_write = color_write | VK_COLOR_COMPONENT_G_BIT;
			}
			if (prs->ColorMask.Contains("B"))
			{
				color_write = color_write | VK_COLOR_COMPONENT_B_BIT;
			}
			if (prs->ColorMask.Contains("A"))
			{
				color_write = color_write | VK_COLOR_COMPONENT_A_BIT;
			}
		}

		VkPipelineColorBlendAttachmentState& blend = att_state[0];
		blend.colorWriteMask = color_write;
		blend.blendEnable = prs->Blend.enable;
		if (prs->Blend.enable)
		{
			blend.colorBlendOp = VK_BLEND_OP_ADD;
			blend.alphaBlendOp = VK_BLEND_OP_ADD;

			const char* strs[] = {
				"One",
				"Zero",
				"SrcColor",
				"SrcAlpha",
				"DstColor",
				"DstAlpha",
				"OneMinusSrcColor",
				"OneMinusSrcAlpha",
				"OneMinusDstColor",
				"OneMinusDstAlpha"
			};
			const int values[] = {
				VK_BLEND_FACTOR_ONE,
				VK_BLEND_FACTOR_ZERO,
				VK_BLEND_FACTOR_SRC_COLOR,
				VK_BLEND_FACTOR_SRC_ALPHA,
				VK_BLEND_FACTOR_DST_COLOR,
				VK_BLEND_FACTOR_DST_ALPHA,
				VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
				VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
				VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA
			};

			const int count = sizeof(values) / sizeof(values[0]);
			for (int i = 0; i < count; i++)
			{
				if (prs->Blend.src == strs[i])
				{
					blend.srcColorBlendFactor = (VkBlendFactor) values[i];
				}

				if (prs->Blend.dst == strs[i])
				{
					blend.dstColorBlendFactor = (VkBlendFactor) values[i];
				}

				if (prs->Blend.src_a == strs[i])
				{
					blend.srcAlphaBlendFactor = (VkBlendFactor) values[i];
				}

				if (prs->Blend.dst_a == strs[i])
				{
					blend.dstAlphaBlendFactor = (VkBlendFactor) values[i];
				}
			}
		}

		Memory::Zero(&ds, sizeof(ds));
		ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		ds.depthTestEnable = VK_TRUE;
		ds.depthWriteEnable = prs->ZWrite == "On";
		ds.depthBoundsTestEnable = VK_FALSE;

		{
			const char* strs[] = {
				"Less",
				"Greater",
				"LEqual",
				"GEqual",
				"Equal",
				"NotEqual",
				"Always"
			};
			const int values[] = {
				VK_COMPARE_OP_LESS,
				VK_COMPARE_OP_GREATER,
				VK_COMPARE_OP_LESS_OR_EQUAL,
				VK_COMPARE_OP_GREATER_OR_EQUAL,
				VK_COMPARE_OP_EQUAL,
				VK_COMPARE_OP_NOT_EQUAL,
				VK_COMPARE_OP_ALWAYS
			};

			const int count = sizeof(values) / sizeof(values[0]);
			for (int i = 0; i < count; i++)
			{
				if (prs->ZTest == strs[i])
				{
					ds.depthCompareOp = (VkCompareOp) values[i];
					break;
				}
			}
		}

		ds.stencilTestEnable = prs->Stencil.enable;
		if (prs->Stencil.enable)
		{
			ds.back.reference = prs->Stencil.RefValue;
			ds.back.compareMask = prs->Stencil.ReadMask;
			ds.back.writeMask = prs->Stencil.WriteMask;

			{
				const char* strs[] = {
					"Less",
					"Greater",
					"LEqual",
					"GEqual",
					"Equal",
					"NotEqual",
					"Always",
					"Never"
				};
				const int values[] = {
					VK_COMPARE_OP_LESS,
					VK_COMPARE_OP_GREATER,
					VK_COMPARE_OP_LESS_OR_EQUAL,
					VK_COMPARE_OP_GREATER_OR_EQUAL,
					VK_COMPARE_OP_EQUAL,
					VK_COMPARE_OP_NOT_EQUAL,
					VK_COMPARE_OP_ALWAYS,
					VK_COMPARE_OP_NEVER
				};

				const int count = sizeof(values) / sizeof(values[0]);
				for (int i = 0; i < count; i++)
				{
					if (prs->Stencil.Comp == strs[i])
					{
						ds.back.compareOp = (VkCompareOp) values[i];
						break;
					}
				}
			}

			{
				const char* strs[] = {
					"Keep",
					"Zero",
					"Replace",
					"IncrSat",
					"DecrSat",
					"Invert",
					"IncrWrap",
					"DecrWrap"
				};
				const int values[] = {
					VK_STENCIL_OP_KEEP,
					VK_STENCIL_OP_ZERO,
					VK_STENCIL_OP_REPLACE,
					VK_STENCIL_OP_INCREMENT_AND_CLAMP,
					VK_STENCIL_OP_DECREMENT_AND_CLAMP,
					VK_STENCIL_OP_INVERT,
					VK_STENCIL_OP_INCREMENT_AND_WRAP,
					VK_STENCIL_OP_DECREMENT_AND_WRAP,
				};

				const int count = sizeof(values) / sizeof(values[0]);
				for (int i = 0; i < count; i++)
				{
					if (prs->Stencil.Pass == strs[i])
					{
						ds.back.passOp = (VkStencilOp) values[i];
					}

					if (prs->Stencil.Fail == strs[i])
					{
						ds.back.failOp = (VkStencilOp) values[i];
					}

					if (prs->Stencil.ZFail == strs[i])
					{
						ds.back.depthFailOp = (VkStencilOp) values[i];
					}
				}
			}
		}
		else
		{
			ds.back.failOp = VK_STENCIL_OP_KEEP;
			ds.back.passOp = VK_STENCIL_OP_KEEP;
			ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
		}
		ds.front = ds.back;

		Memory::Zero(&ms, sizeof(ms));
		ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		ms.pSampleMask = NULL;
		ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		Memory::Zero(shader_pass.shader_stages, sizeof(shader_pass.shader_stages));
		VkPipelineShaderStageCreateInfo* shader_stages = shader_pass.shader_stages;

		shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shader_stages[0].module = vs;
		shader_stages[0].pName = "main";

		shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shader_stages[1].module = ps;
		shader_stages[1].pName = "main";

		pipeline_info.stageCount = 2;
		pipeline_info.pStages = shader_stages;
		pipeline_info.renderPass = NULL;
		pipeline_info.pVertexInputState = &vi;
		pipeline_info.pInputAssemblyState = &ia;
		pipeline_info.pDepthStencilState = &ds;
		pipeline_info.pRasterizationState = &rs;
		pipeline_info.pColorBlendState = &cb;
		pipeline_info.pMultisampleState = &ms;
		pipeline_info.pViewportState = &vp;
		pipeline_info.pDynamicState = &dynamic_state;
	}

	ShaderVulkan::ShaderVulkan()
	{
	}

	ShaderVulkan::~ShaderVulkan()
	{
		auto display = (DisplayVulkan*) Graphics::GetDisplay();
		auto device = display->GetDevice();

		for (auto& i : m_pixel_shaders)
		{
			vkDestroyShaderModule(device, i.second, NULL);
		}
		m_pixel_shaders.Clear();
		for (auto& i : m_vertex_shaders)
		{
			vkDestroyShaderModule(device, i.second, NULL);
		}
		m_vertex_shaders.Clear();
		for (auto& i : m_passes)
		{
			for (auto& j : i.pipelines)
			{
				vkDestroyPipeline(device, j.second, NULL);
			}
			i.pipelines.Clear();

			vkDestroyPipelineLayout(device, i.pipeline_layout, NULL);
			if (i.descriptor_pool)
			{
				vkDestroyDescriptorPool(device, i.descriptor_pool, NULL);
			}
			vkDestroyDescriptorSetLayout(device, i.descriptor_layout, NULL);
		}
		m_passes.Clear();
	}

	void ShaderVulkan::Compile()
	{
		this->CreateShaders();
		this->CreatePasses();
	}

	static const String g_shader_header =
		"#version 310 es\n"
		"#extension GL_ARB_separate_shader_objects : enable\n"
		"#extension GL_ARB_shading_language_420pack : enable\n"
		"#define VR_VULKAN 1\n"
		"#define UniformPush(exp1) layout(exp1)\n"
		"#define UniformBuffer(exp1, exp2) layout(exp1, exp2)\n"
		"#define UniformTexture(exp1) layout(exp1)\n"
		"#define Varying(exp1) layout(exp1)\n";

	static String combine_shader_src(const Vector<String>& includes, const String& src)
	{
		String source = g_shader_header;
		for (const auto& i : includes)
		{
			auto include_path = Application::DataPath() + "/shader/Include/" + i;
			auto bytes = File::ReadAllBytes(include_path);
			auto include_str = String(bytes);
			source += include_str + "\n";
		}
		source += src;

		return source;
	}

	void ShaderVulkan::CreateShaders()
	{
		auto display = (DisplayVulkan*) Graphics::GetDisplay();
		auto device = display->GetDevice();

		const auto& xml = ((Shader*) this)->m_xml;

		for (const auto& i : xml.vss)
		{
			auto source = combine_shader_src(i.includes, i.src);

			Vector<unsigned int> spirv;
			compile_with_cache(spirv, source, VK_SHADER_STAGE_VERTEX_BIT);

			VkShaderModule module = create_shader_module(device, &spirv[0], spirv.SizeInBytes());

			m_vertex_shaders.Add(i.name, module);
		}

		for (const auto& i : xml.pss)
		{
			auto source = combine_shader_src(i.includes, i.src);

			Vector<unsigned int> spirv;
			compile_with_cache(spirv, source, VK_SHADER_STAGE_FRAGMENT_BIT);

			VkShaderModule module = create_shader_module(device, &spirv[0], spirv.SizeInBytes());

			m_pixel_shaders.Add(i.name, module);
		}
	}

	void ShaderVulkan::CreatePasses()
	{
		const auto& xml = ((Shader*) this)->m_xml;

		m_passes.Resize(xml.passes.Size());
		for (int i = 0; i < xml.passes.Size(); i++)
		{
			auto& xml_pass = xml.passes[i];
			auto& pass = m_passes[i];

			pass.name = xml_pass.name;
			create_descriptor_set_info(xml_pass, xml, pass);
			pass.pipeline_layout = create_pipeline_layout(pass.descriptor_layout);
			prepare_pipeline(xml_pass, xml, pass, m_vertex_shaders[xml_pass.vs], m_pixel_shaders[xml_pass.ps]);
		}
	}

	void ShaderVulkan::ClearPipelines()
	{
		auto display = (DisplayVulkan*) Graphics::GetDisplay();
		auto device = display->GetDevice();

		for (auto& i : m_passes)
		{
			for (auto& j : i.pipelines)
			{
				vkDestroyPipeline(device, j.second, NULL);
			}
			i.pipelines.Clear();
		}
	}

	void ShaderVulkan::PreparePass(int index)
	{
		auto display = (DisplayVulkan*) Graphics::GetDisplay();
		auto device = display->GetDevice();
		auto& pass = m_passes[index];
		auto render_pass = RenderPass::GetRenderPassBinding()->GetVkRenderPass();

		if (!pass.pipelines.Contains(render_pass))
		{
			VkPipeline pipeline;
			pass.pipeline_info.renderPass = render_pass;
			VkResult err = vkCreateGraphicsPipelines(device, display->GetPipelineCache(), 1, &pass.pipeline_info, NULL, &pipeline);
			assert(!err);

			pass.pipelines.Add(render_pass, pipeline);
		}
	}

	void ShaderVulkan::BindMaterial(int index, const Ref<Material>& material, int lightmap_index)
	{
		auto display = (DisplayVulkan*) Graphics::GetDisplay();
		auto& pass = m_passes[index];
		auto& descriptor_sets = RefCast<MaterialVulkan>(material)->GetDescriptorSet(index);
		VkCommandBuffer cmd = display->GetCurrentDrawCommand();

		lightmap_index = Mathf::Clamp(lightmap_index, 0, LIGHT_MAP_COUNT_MAX - 1);

		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
			pass.pipeline_layout, 0, 1, &descriptor_sets[lightmap_index], 0, NULL);
	}

	void ShaderVulkan::PushConstant(int index, void* data, int size)
	{
		auto display = (DisplayVulkan*) Graphics::GetDisplay();
		auto& pass = m_passes[index];
		VkCommandBuffer cmd = display->GetCurrentDrawCommand();

		vkCmdPushConstants(cmd, pass.pipeline_layout,
			VK_SHADER_STAGE_VERTEX_BIT, 0, size,
			data);
	}

	void ShaderVulkan::BeginPass(int index)
	{
		auto display = (DisplayVulkan*) Graphics::GetDisplay();
		auto device = display->GetDevice();
		auto& pass = m_passes[index];
		auto render_pass = RenderPass::GetRenderPassBinding();
		VkCommandBuffer cmd = display->GetCurrentDrawCommand();

		VkPipeline pipeline = pass.pipelines[render_pass->GetVkRenderPass()];

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		int width = render_pass->GetFrameBufferWidth();
		int height = render_pass->GetFrameBufferHeight();
		auto rect = render_pass->GetRect();

		VkViewport viewport;
		Memory::Zero(&viewport, sizeof(viewport));
		viewport.width = rect.width * width;
		viewport.height = rect.height * height;
		viewport.x = rect.x * width;
		viewport.y = rect.y * height;
		viewport.minDepth = (float) 0.0f;
		viewport.maxDepth = (float) 1.0f;
		vkCmdSetViewport(cmd, 0, 1, &viewport);

		VkRect2D scissor;
		Memory::Zero(&scissor, sizeof(scissor));
		scissor.extent.width = width;
		scissor.extent.height = height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(cmd, 0, 1, &scissor);
	}

	void ShaderVulkan::EndPass(int index)
	{

	}
}

#endif
