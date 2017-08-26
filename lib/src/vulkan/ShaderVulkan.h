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

	class Material;

	class ShaderVulkan : public Object
	{
	public:
		ShaderVulkan();
		virtual ~ShaderVulkan();
		int GetPassCount() const { return m_passes.Size(); }
		void ClearPipelines();
		void PreparePass(int index);
		void BeginPass(int index);
		void BindSharedMaterial(int index, const Ref<Material>& material) { }
		void BindMaterial(int index, const Ref<Material>& material, int lightmap_index);
		void BindLightmap(int index, const Ref<Material>& material, int lightmap_index) { }
		void PushConstant(int index, void* data, int size);
		void EndPass(int index);

		Vector<VkDescriptorSet> CreateDescriptorSet(int index);
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
	};
}