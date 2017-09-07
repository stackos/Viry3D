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
#include "vulkan_include.h"
#include "graphics/Texture.h"
#include "graphics/UniformBuffer.h"
#include "math/Matrix4x4.h"

namespace Viry3D
{
#define DESCRIPTOR_POOL_SIZE_MAX 65536

	struct ShaderPass
	{
		String name;
		Vector<VkDescriptorSetLayoutBinding> binds;
		VkDescriptorSetLayout descriptor_layout;
		VkDescriptorPool descriptor_pool;
		VkPipelineLayout pipeline_layout;

		Map<VkRenderPass, VkPipeline> pipelines;
		VkGraphicsPipelineCreateInfo pipeline_info;
		VkPipelineDynamicStateCreateInfo dynamic_state;
		VkDynamicState dynamic_state_enables[VK_DYNAMIC_STATE_RANGE_SIZE];
		VkPipelineVertexInputStateCreateInfo vi;
		VkPipelineInputAssemblyStateCreateInfo ia;
		VkPipelineRasterizationStateCreateInfo rs;
		VkPipelineColorBlendStateCreateInfo cb;
		VkPipelineDepthStencilStateCreateInfo ds;
		VkPipelineViewportStateCreateInfo vp;
		VkPipelineMultisampleStateCreateInfo ms;
		VkPipelineShaderStageCreateInfo shader_stages[2];
		VkVertexInputBindingDescription vi_bindings[1];
		Vector<VkVertexInputAttributeDescription> vi_attrs;
		VkPipelineColorBlendAttachmentState att_state[1];

		Vector<VkDescriptorBufferInfo> uniform_infos;
		Vector<VkDescriptorImageInfo> sampler_infos;
		Vector<const void*> uniform_xmls;
		Vector<VkWriteDescriptorSet> uniform_writes;
	};

	struct RendererDescriptor
	{
		VkDescriptorSetLayout layout;
		VkDescriptorPool pool;
	};

	class Material;
	class DescriptorSet;

	class ShaderVulkan: public Object
	{
	public:
		ShaderVulkan();
		virtual ~ShaderVulkan();
		int GetPassCount() const { return m_passes.Size(); }
		void ClearPipelines();
		void PreparePass(int index);
		void UpdateRendererDescriptorSet(Ref<DescriptorSet>& renderer_descriptor_set, Ref<UniformBuffer>& descriptor_set_buffer, const void* data, int size, int lightmap_index);
		void BeginPass(int index);
		void BindSharedMaterial(int index, const Ref<Material>& material) { }
		void BindMaterial(int index, const Ref<Material>& material, int lightmap_index, const Ref<DescriptorSet>& renderer_descriptor_set);
		void BindRendererDescriptorSet(int index, const Ref<Material>& material, Ref<UniformBuffer>& descriptor_set_buffer, int lightmap_index) { }
		void EndPass(int index);

		Vector<VkDescriptorSet> CreateDescriptorSet(int index);
		VkDescriptorSet CreateRendererDescriptorSet();
		Ref<UniformBuffer> CreateUniformBuffer(int index);
		Vector<VkWriteDescriptorSet>& GetDescriptorSetWriteInfo(int index);
		const Vector<const void*>& GetUniformXmls(int index);

	protected:
		void Compile();

	private:
		void CreateShaders();
		void CreatePasses();

		Vector<ShaderPass> m_passes;
		Map<String, VkShaderModule> m_vertex_shaders;
		Map<String, VkShaderModule> m_pixel_shaders;
		RendererDescriptor m_renderer_descriptor;
	};
}
