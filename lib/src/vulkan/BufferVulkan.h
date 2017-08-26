#pragma once

#include "vulkan_include.h"
#include "graphics/BufferType.h"
#include "memory/ByteBuffer.h"
#include <functional>

namespace Viry3D
{
	class BufferVulkan
	{
	public:
		typedef std::function<void(void* param, const ByteBuffer& buffer)> FillFunc;
		void Fill(void* param, FillFunc fill);
		VkBuffer GetBuffer() const { return m_buffer; }
		VkDeviceMemory GetMemory() const { return m_memory; }
		int GetSize() const { return m_size; }

	protected:
		BufferVulkan();
		virtual ~BufferVulkan();
		void CreateInternal(BufferType type, bool dynamic = false);

		int m_size;

	private:
		BufferType m_type;
		VkBuffer m_buffer;
		VkDeviceMemory m_memory;
	};
}