#include "BufferVulkan.h"
#include "DisplayVulkan.h"
#include "graphics/Graphics.h"
#include "memory/Memory.h"

namespace Viry3D
{
	BufferVulkan::BufferVulkan():
		m_type(BufferType::None),
		m_size(0),
		m_buffer(NULL),
		m_memory(NULL)
	{
	}

	BufferVulkan::~BufferVulkan()
	{
		auto display = (DisplayVulkan*) Graphics::GetDisplay();
		auto device = display->GetDevice();

		vkDeviceWaitIdle(device);

		vkFreeMemory(device, m_memory, NULL);
		vkDestroyBuffer(device, m_buffer, NULL);
	}

	void BufferVulkan::CreateInternal(BufferType type, bool dynamic)
	{
		auto display = (DisplayVulkan*) Graphics::GetDisplay();
		auto device = display->GetDevice();
		VkResult err;

		if(m_buffer == NULL)
		{
			VkBufferCreateInfo buf_info;
			Memory::Zero(&buf_info, sizeof(buf_info));
			buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buf_info.pNext = NULL;
			buf_info.size = m_size;
			buf_info.flags = 0;

			m_type = type;
			switch(type)
			{
				case BufferType::Vertex:
				{
					buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
					break;
				}
				case BufferType::Index:
				{
					buf_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
					break;
				}
				case BufferType::Uniform:
				{
					buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
					break;
				}
				case BufferType::Image:
				{
					buf_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
					break;
				}
			}

			err = vkCreateBuffer(device, &buf_info, NULL, &m_buffer);
			assert(!err);
		}

		if(m_memory == NULL)
		{
			VkMemoryRequirements mem_reqs;
			vkGetBufferMemoryRequirements(device, m_buffer, &mem_reqs);

			VkMemoryAllocateInfo mem_alloc = {
				VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				NULL,
				mem_reqs.size,
				0,
			};
			bool pass = display->CheckMemoryType(
				mem_reqs.memoryTypeBits,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				&mem_alloc.memoryTypeIndex);
			assert(pass);

			err = vkAllocateMemory(device, &mem_alloc, NULL, &m_memory);
			assert(!err);

			err = vkBindBufferMemory(device, m_buffer, m_memory, 0);
			assert(!err);
		}
	}

	void BufferVulkan::Fill(void* param, FillFunc fill)
	{
		auto display = (DisplayVulkan*) Graphics::GetDisplay();
		auto device = display->GetDevice();
		VkResult err;

		void* data;
		err = vkMapMemory(device, m_memory, 0, m_size, 0, &data);
		assert(!err);

		ByteBuffer buffer((byte*) data, m_size);
		fill(param, buffer);

		vkUnmapMemory(device, m_memory);
	}
}