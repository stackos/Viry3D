#pragma once

#include "Object.h"
#include "vulkan_include.h"
#include "graphics/UniformBuffer.h"

namespace Viry3D
{
	class Texture;

	class MaterialVulkan : public Object
	{
	public:
		virtual ~MaterialVulkan();
		const Vector<VkDescriptorSet>& GetDescriptorSet(int pass_index);

	protected:
		MaterialVulkan();
		void UpdateUniformsBegin(int pass_index);
		void UpdateUniformsEnd(int pass_index);
		void* SetUniformBegin(int pass_index);
		void SetUniformEnd(int pass_index);
		void SetUniform(int pass_index, void* uniform_buffer, const String& name, void* data, int size);
		void SetUniformTexture(int pass_index, const String& name, const Texture* texture);

	private:
		bool CheckWritesDirty(int pass_index) const;

	private:
		Vector<Vector<VkDescriptorSet>> m_descriptor_sets;
		Vector<Ref<UniformBuffer>> m_uniform_buffers;
		Vector<VkWriteDescriptorSet> m_writes_old;
	};
}