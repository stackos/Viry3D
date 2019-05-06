/*
* Viry3D
* Copyright 2014-2019 by Stack - stackos@qq.com
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
#if VR_VULKAN
            m_buffer(VK_NULL_HANDLE),
            m_memory(VK_NULL_HANDLE),
            m_buffer_view(VK_NULL_HANDLE),
            m_device_local(false),
#elif VR_GLES
            m_buffer(0),
            m_target(0),
            m_usage(0),
#endif
            m_size(size)
        {
#if VR_VULKAN
            Memory::Zero(&m_memory_info, sizeof(m_memory_info));
#endif
        }

        int GetSize() const { return m_size; }

#if VR_VULKAN
        void Destroy(VkDevice device)
        {
            if (m_buffer_view)
            {
                vkDestroyBufferView(device, m_buffer_view, nullptr);
            }
            vkDestroyBuffer(device, m_buffer, nullptr);
            vkFreeMemory(device, m_memory, nullptr);
        }

        const VkBuffer& GetBuffer() const { return m_buffer; }
        const VkDeviceMemory& GetMemory() const { return m_memory; }
        const VkBufferView& GetBufferView() const { return m_buffer_view; }
        bool IsDeviceLocal() const { return m_device_local; }
#elif VR_GLES
        ~BufferObject()
        {
            if (m_buffer)
            {
                glDeleteBuffers(1, &m_buffer);
            }
        }

        GLuint GetBuffer() const { return m_buffer; }
        GLenum GetTarget() const { return m_target; }
        GLenum GetUsage() const { return m_usage; }
        void Bind() const { glBindBuffer(m_target, m_buffer); }
        void Unind() const { glBindBuffer(m_target, 0); }
#endif

    private:
#if VR_VULKAN
        VkBuffer m_buffer;
        VkDeviceMemory m_memory;
        VkMemoryAllocateInfo m_memory_info;
        VkBufferView m_buffer_view;
        bool m_device_local;
#elif VR_GLES
        GLuint m_buffer;
        GLenum m_target;
        GLenum m_usage;
#endif
        int m_size;
    };
}
