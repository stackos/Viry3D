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
    struct BufferObject
    {
        VkBuffer buffer;
        VkDeviceMemory memory;
        VkMemoryAllocateInfo memory_info;
        int size;

        BufferObject():
            buffer(VK_NULL_HANDLE),
            memory(VK_NULL_HANDLE),
            size(0)
        {
            Memory::Zero(&memory_info, sizeof(memory_info));
        }

        void Destroy(VkDevice device)
        {
            vkDestroyBuffer(device, buffer, nullptr);
            vkFreeMemory(device, memory, nullptr);
        }
    };
}
