/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "vulkan/VulkanBuffer.h"

#include <utils/Panic.h>

namespace filament {
namespace backend {

VulkanBuffer::VulkanBuffer(VulkanContext& context, VulkanStagePool& stagePool,
        VkBufferUsageFlags usage, uint32_t numBytes) : mContext(context), mStagePool(stagePool) {
    // Create the VkBuffer.
    VkBufferCreateInfo bufferInfo {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		nullptr,
		0,
        numBytes,
        usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT
    };
    VmaAllocationCreateInfo allocInfo {
		0,
        VMA_MEMORY_USAGE_GPU_ONLY
    };
    vmaCreateBuffer(context.allocator, &bufferInfo, &allocInfo, &mGpuBuffer, &mGpuMemory, nullptr);
}

VulkanBuffer::~VulkanBuffer() {
    vmaDestroyBuffer(mContext.allocator, mGpuBuffer, mGpuMemory);
}

void VulkanBuffer::loadFromCpu(const void* cpuData, uint32_t byteOffset, uint32_t numBytes) {
    assert(byteOffset == 0);
    VulkanStage const* stage = mStagePool.acquireStage(numBytes);
    void* mapped;
    vmaMapMemory(mContext.allocator, stage->memory, &mapped);
    memcpy(mapped, cpuData, numBytes);
    vmaUnmapMemory(mContext.allocator, stage->memory);
    vmaFlushAllocation(mContext.allocator, stage->memory, byteOffset, numBytes);

    auto copyToDevice = [this, numBytes, stage] (VulkanCommandBuffer& commands) {
        VkBufferCopy region { 0, 0, numBytes };
        vkCmdCopyBuffer(commands.cmdbuffer, stage->buffer, mGpuBuffer, 1, &region);

        // Ensure that the copy finishes before the next draw call.
        VkBufferMemoryBarrier barrier {
            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
			nullptr,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            mGpuBuffer,
			0,
            VK_WHOLE_SIZE
        };
        vkCmdPipelineBarrier(commands.cmdbuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr, 1, &barrier, 0, nullptr);

        mStagePool.releaseStage(stage, commands);
    };

    // If inside beginFrame / endFrame, use the swap context, otherwise use the work cmdbuffer.
    if (mContext.currentCommands) {
        copyToDevice(*mContext.currentCommands);
    } else {
        acquireWorkCommandBuffer(mContext);
        copyToDevice(mContext.work);
        flushWorkCommandBuffer(mContext);
    }
}

} // namespace filament
} // namespace backend
