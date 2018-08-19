/*
* Viry3D
* Copyright 2014-2018 by Stack - stackos@qq.com
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

#include "Display.h"
#include "memory/Memory.h"

namespace Viry3D
{
    class BufferObject
    {
    private:
        friend class DisplayPrivate;

    public:
        BufferObject(int size):
            m_buffer(VK_NULL_HANDLE),
            m_memory(VK_NULL_HANDLE),
            m_size(size)
        {
            Memory::Zero(&m_memory_info, sizeof(m_memory_info));
        }

        void Destroy(VkDevice device)
        {
            vkDestroyBuffer(device, m_buffer, nullptr);
            vkFreeMemory(device, m_memory, nullptr);
        }

        const VkBuffer& GetBuffer() const { return m_buffer; }
        const VkDeviceMemory& GetMemory() const { return m_memory; }
        int GetSize() const { return m_size; }

    private:
        VkBuffer m_buffer;
        VkDeviceMemory m_memory;
        VkMemoryAllocateInfo m_memory_info;
        int m_size;
    };
}
