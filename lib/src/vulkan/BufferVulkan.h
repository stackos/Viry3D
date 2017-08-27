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
