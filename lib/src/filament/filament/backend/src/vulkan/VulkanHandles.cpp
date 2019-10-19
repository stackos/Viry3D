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

#include "vulkan/VulkanHandles.h"

#include "DataReshaper.h"

#include <utils/Panic.h>
#include <algorithm>

#define FILAMENT_VULKAN_VERBOSE 0

namespace filament {
namespace backend {

static void flipVertically(VkRect2D* rect, uint32_t framebufferHeight) {
    rect->offset.y = framebufferHeight - rect->offset.y - rect->extent.height;
}

static void flipVertically(VkViewport* rect, uint32_t framebufferHeight) {
    rect->y = framebufferHeight - rect->y - rect->height;
}

static void clampToFramebuffer(VkRect2D* rect, uint32_t fbWidth, uint32_t fbHeight) {
    int32_t x = std::max(rect->offset.x, 0);
    int32_t y = std::max(rect->offset.y, 0);
    int32_t right = std::min(rect->offset.x + (int32_t) rect->extent.width, (int32_t) fbWidth);
    int32_t top = std::min(rect->offset.y + (int32_t) rect->extent.height, (int32_t) fbHeight);
    rect->offset.x = x;
    rect->offset.y = y;
    rect->extent.width = right - x;
    rect->extent.height = top - y;
}

VulkanProgram::VulkanProgram(VulkanContext& context, const Program& builder) noexcept :
        HwProgram(builder.getName()), context(context) {
    auto const& blobs = builder.getShadersSource();
    VkShaderModule* modules[2] = { &bundle.vertex, &bundle.fragment };
    bool missing = false;
    for (size_t i = 0; i < Program::SHADER_TYPE_COUNT; i++) {
        const auto& blob = blobs[i];
        VkShaderModule* module = modules[i];
        if (blob.empty()) {
            missing = true;
            continue;
        }
        VkShaderModuleCreateInfo moduleInfo = {};
        moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleInfo.codeSize = blob.size();
        moduleInfo.pCode = (uint32_t*) blob.data();
        VkResult result = vkCreateShaderModule(context.device, &moduleInfo, VKALLOC, module);
        ASSERT_POSTCONDITION(result == VK_SUCCESS, "Unable to create shader module.");
    }

    // Output a warning because it's okay to encounter empty blobs, but it's not okay to use
    // this program handle in a draw call.
    if (missing) {
        utils::slog.w << "Missing SPIR-V shader: " << builder.getName().c_str() << utils::io::endl;
        return;
    }

    // Make a copy of the binding map
    samplerGroupInfo = builder.getSamplerGroupInfo();
#if FILAMENT_VULKAN_VERBOSE
    utils::slog.d << "Created VulkanProgram " << builder.getName().c_str()
                << ", variant = (" << utils::io::hex
                << builder.getVariant() << utils::io::dec << "), "
                << "shaders = (" << bundle.vertex << ", " << bundle.fragment << ")"
                << utils::io::endl;
#endif
}

VulkanProgram::~VulkanProgram() {
    vkDestroyShaderModule(context.device, bundle.vertex, VKALLOC);
    vkDestroyShaderModule(context.device, bundle.fragment, VKALLOC);
}

VulkanRenderTarget::~VulkanRenderTarget() {
    if (!mSharedColorImage) {
        vkDestroyImageView(mContext.device, mColor.view, VKALLOC);
        vkDestroyImage(mContext.device, mColor.image, VKALLOC);
        vkFreeMemory(mContext.device, mColor.memory, VKALLOC);
    }
    if (!mSharedDepthImage) {
        vkDestroyImageView(mContext.device, mDepth.view, VKALLOC);
        vkDestroyImage(mContext.device, mDepth.image, VKALLOC);
        vkFreeMemory(mContext.device, mDepth.memory, VKALLOC);
    }
}

void VulkanRenderTarget::transformClientRectToPlatform(VkRect2D* bounds) const {
    // For the backbuffer, there are corner cases where the platform's surface resolution does not
    // match what Filament expects, so we need to make an appropriate transformation (e.g. create a
    // VkSurfaceKHR on a high DPI display, then move it to a low DPI display).
    if (!mOffscreen) {
        const VkExtent2D platformSize = mContext.currentSurface->surfaceCapabilities.currentExtent;
        const VkExtent2D clientSize = mContext.currentSurface->clientSize;

        // Because these types of coordinates are pixel-addressable, we purposefully use integer
        // math and rely on left-to-right evaluation.
        bounds->offset.x = bounds->offset.x * platformSize.width / clientSize.width;
        bounds->offset.y = bounds->offset.y * platformSize.height / clientSize.height;
        bounds->extent.width = bounds->extent.width * platformSize.width / clientSize.width;
        bounds->extent.height = bounds->extent.height * platformSize.height / clientSize.height;
    }
    const auto& extent = getExtent();
    flipVertically(bounds, extent.height);
    clampToFramebuffer(bounds, extent.width, extent.height);
}

void VulkanRenderTarget::transformClientRectToPlatform(VkViewport* bounds) const {
    // For the backbuffer, we must check if platform size and client size differ, then scale
    // appropriately. Note the +2 correction factor. This prevents the platform from lerping pixels
    // along the edge of the viewport with pixels that live outside the viewport. Luckily this
    // correction factor only applies in obscure conditions (e.g. after dragging a high-DPI window
    // to a low-DPI display).
    if (!mOffscreen) {
        const VkExtent2D platformSize = mContext.currentSurface->surfaceCapabilities.currentExtent;
        const VkExtent2D clientSize = mContext.currentSurface->clientSize;
        if (platformSize.width != clientSize.width) {
            const float xscale = float(platformSize.width + 2) / float(clientSize.width);
            bounds->x *= xscale;
            bounds->width *= xscale;
        }
        if (platformSize.height != clientSize.height) {
            const float yscale = float(platformSize.height + 2) / float(clientSize.height);
            bounds->y *= yscale;
            bounds->height *= yscale;
        }
    }
    flipVertically(bounds, getExtent().height);
}

VkExtent2D VulkanRenderTarget::getExtent() const {
    if (mOffscreen) {
        return {width, height};
    }
    return mContext.currentSurface->surfaceCapabilities.currentExtent;
}

VulkanAttachment VulkanRenderTarget::getColor() const {
    return mOffscreen ? mColor : getSwapContext(mContext).attachment;
}

VulkanAttachment VulkanRenderTarget::getDepth() const {
    return mDepth;
}

void VulkanRenderTarget::createDepthImage(VkFormat format) {
	VkExtent2D extent;
	if (mOffscreen) {
		extent.width = width;
		extent.height = height;
	} else {
		extent = mContext.currentSurface->surfaceCapabilities.currentExtent;
	}
    this->mDepth.format = format;
    mSharedDepthImage = false;
    // Create an appropriately-sized device-only VkImage for the depth attachment.
    // TODO: for depth, can we re-use the image associated with the swap chain?
	VkImageCreateInfo depthImageInfo { };
	depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
	depthImageInfo.extent = { extent.width, extent.height, 1 };
	depthImageInfo.format = mDepth.format;
	depthImageInfo.mipLevels = 1;
	depthImageInfo.arrayLayers = 1;
	depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    VkResult error = vkCreateImage(mContext.device, &depthImageInfo, VKALLOC, &mDepth.image);
    ASSERT_POSTCONDITION(!error, "Unable to create depth attachment.");

    // Allocate memory for the depth image and bind it.
    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(mContext.device, mDepth.image, &memReqs);
    VkMemoryAllocateInfo depthAllocInfo {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		nullptr,
        memReqs.size,
        selectMemoryType(mContext, memReqs.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    };
    error = vkAllocateMemory(mContext.device, &depthAllocInfo, nullptr, &mDepth.memory);
    ASSERT_POSTCONDITION(!error, "Unable to allocate depth memory.");
    error = vkBindImageMemory(mContext.device, mDepth.image, mDepth.memory, 0);
    ASSERT_POSTCONDITION(!error, "Unable to bind depth memory.");

    // Transition the depth image into an optimal layout and assume there's no need to read from it.
	VkImageMemoryBarrier depthBarrier { };
	depthBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	depthBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	depthBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	depthBarrier.image = mDepth.image;
	depthBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	depthBarrier.subresourceRange.levelCount = 1;
	depthBarrier.subresourceRange.layerCount = 1;
	depthBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    vkCmdPipelineBarrier(mContext.currentCommands->cmdbuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, nullptr, 0, nullptr, 1,
            &depthBarrier);

    // Create a VkImageView so that we can attach it to the framebuffer.
    VkImageViewCreateInfo depthViewInfo {
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		nullptr,
		0,
		mDepth.image,
		VK_IMAGE_VIEW_TYPE_2D,
		mDepth.format,
		{
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY
		},
		{
			VK_IMAGE_ASPECT_DEPTH_BIT,
			0,
			1,
			0,
			1
		}
    };
    error = vkCreateImageView(mContext.device, &depthViewInfo, VKALLOC, &mDepth.view);
    ASSERT_POSTCONDITION(!error, "Unable to create depth attachment view.");
}

void VulkanRenderTarget::setColorImage(VulkanAttachment c) {
    assert(mOffscreen);
    mColor = c;
}

void VulkanRenderTarget::setDepthImage(VulkanAttachment d) {
    assert(mOffscreen);
    mDepth = d;
}

VulkanVertexBuffer::VulkanVertexBuffer(VulkanContext& context, VulkanStagePool& stagePool,
        uint8_t bufferCount, uint8_t attributeCount, uint32_t elementCount,
        AttributeArray const& attributes) :
        HwVertexBuffer(bufferCount, attributeCount, elementCount, attributes) {
    buffers.reserve(bufferCount);
    for (uint8_t bufferIndex = 0; bufferIndex < bufferCount; ++bufferIndex) {
        uint32_t size = 0;
        for (auto const& item : attributes) {
            if (item.buffer == bufferIndex) {
              uint32_t end = item.offset + elementCount * item.stride;
                size = std::max(size, end);
            }
        }
        buffers.emplace_back(new VulkanBuffer(context, stagePool, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                size));
    }
}

VulkanUniformBuffer::VulkanUniformBuffer(VulkanContext& context, VulkanStagePool& stagePool,
        uint32_t numBytes, backend::BufferUsage usage)
        : mContext(context), mStagePool(stagePool) {
    // Create the VkBuffer.
    VkBufferCreateInfo bufferInfo {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		nullptr,
		0,
        numBytes,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    };
    VmaAllocationCreateInfo allocInfo {
		0,
        VMA_MEMORY_USAGE_GPU_ONLY
    };
    vmaCreateBuffer(mContext.allocator, &bufferInfo, &allocInfo, &mGpuBuffer, &mGpuMemory, nullptr);
}

void VulkanUniformBuffer::loadFromCpu(const void* cpuData, uint32_t numBytes) {
    VulkanStage const* stage = mStagePool.acquireStage(numBytes);
    void* mapped;
    vmaMapMemory(mContext.allocator, stage->memory, &mapped);
    memcpy(mapped, cpuData, numBytes);
    vmaUnmapMemory(mContext.allocator, stage->memory);
    vmaFlushAllocation(mContext.allocator, stage->memory, 0, numBytes);

    auto copyToDevice = [this, numBytes, stage] (VulkanCommandBuffer& commands) {
        VkBufferCopy region { 0, 0, numBytes };
        vkCmdCopyBuffer(commands.cmdbuffer, stage->buffer, mGpuBuffer, 1, &region);

        // Ensure that the copy finishes before the next draw call.
		VkBufferMemoryBarrier barrier { };
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.buffer = mGpuBuffer;
		barrier.size = VK_WHOLE_SIZE;

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

VulkanUniformBuffer::~VulkanUniformBuffer() {
    vmaDestroyBuffer(mContext.allocator, mGpuBuffer, mGpuMemory);
}

VulkanTexture::VulkanTexture(VulkanContext& context, SamplerType target, uint8_t levels,
        TextureFormat tformat, uint8_t samples, uint32_t w, uint32_t h, uint32_t depth,
        TextureUsage usage, VulkanStagePool& stagePool) :
        HwTexture(target, levels, samples, w, h, depth, tformat, usage),
        vkformat(getVkFormat(tformat)), mContext(context), mStagePool(stagePool) {

    // Vulkan does not support 24-bit depth, use the official fallback format.
    if (tformat == TextureFormat::DEPTH24) {
        vkformat = mContext.depthFormat;
    }

    // Create an appropriately-sized device-only VkImage, but do not fill it yet.
	VkImageCreateInfo imageInfo { };
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = w;
	imageInfo.extent.height = h;
	imageInfo.extent.depth = depth;
	imageInfo.format = vkformat;
	imageInfo.mipLevels = levels;
	imageInfo.arrayLayers = 1;
	imageInfo.usage = 0;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    if (target == SamplerType::SAMPLER_CUBEMAP) {
        imageInfo.arrayLayers = 6;
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }
    if (usage & TextureUsage::SAMPLEABLE) {
        imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if (usage & TextureUsage::COLOR_ATTACHMENT) {
        imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
	if (usage & TextureUsage::DEPTH_ATTACHMENT) {
		imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
    if (usage & TextureUsage::STENCIL_ATTACHMENT) {
        imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    if (usage & TextureUsage::UPLOADABLE) {
        // Uploadable textures can be used as a blit source (e.g. for mipmap generation)
        // therefore we must set both the TRANSFER_DST and TRANSFER_SRC flags.
        imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    VkResult error = vkCreateImage(context.device, &imageInfo, VKALLOC, &textureImage);
    if (error) {
        utils::slog.d << "vkCreateImage: "
            << "result = " << error << ", "
            << "extent = " << w << "x" << h << "x"<< depth << ", "
            << "mipLevels = " << levels << ", "
            << "format = " << vkformat << utils::io::endl;
    }
    ASSERT_POSTCONDITION(!error, "Unable to create image.");

    // Allocate memory for the VkImage and bind it.
    VkMemoryRequirements memReqs = {};
    vkGetImageMemoryRequirements(context.device, textureImage, &memReqs);
    VkMemoryAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		nullptr,
        memReqs.size,
        selectMemoryType(context, memReqs.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    };
    error = vkAllocateMemory(context.device, &allocInfo, nullptr, &textureImageMemory);
    ASSERT_POSTCONDITION(!error, "Unable to allocate image memory.");
    error = vkBindImageMemory(context.device, textureImage, textureImageMemory, 0);
    ASSERT_POSTCONDITION(!error, "Unable to bind image.");

    // Create a VkImageView so that shaders can sample from the image.
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = textureImage;
    viewInfo.format = vkformat;
    viewInfo.subresourceRange.aspectMask = 0;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = levels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    if (target == SamplerType::SAMPLER_CUBEMAP) {
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        viewInfo.subresourceRange.layerCount = 6;
    } else {
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.subresourceRange.layerCount = 1;
    }
	if ((usage & TextureUsage::DEPTH_ATTACHMENT) || (usage & TextureUsage::STENCIL_ATTACHMENT)) {
		if (usage & TextureUsage::DEPTH_ATTACHMENT) {
			viewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
		}
		if (usage & TextureUsage::STENCIL_ATTACHMENT) {
			viewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	} else {
		viewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
	}
    error = vkCreateImageView(context.device, &viewInfo, VKALLOC, &imageView);
    ASSERT_POSTCONDITION(!error, "Unable to create image view.");
}

VulkanTexture::~VulkanTexture() {
    vkDestroyImage(mContext.device, textureImage, VKALLOC);
    vkDestroyImageView(mContext.device, imageView, VKALLOC);
    vkFreeMemory(mContext.device, textureImageMemory, VKALLOC);
}

void VulkanTexture::updateTexture(
	const PixelBufferDescriptor& data,
	int layer, int level,
	int x, int y,
	int w, int h)
{
	VulkanStage const* stage = mStagePool.acquireStage((uint32_t) data.size);
	void* mapped;
	vmaMapMemory(mContext.allocator, stage->memory, &mapped);
	memcpy(mapped, data.buffer, data.size);
	vmaUnmapMemory(mContext.allocator, stage->memory);
	vmaFlushAllocation(mContext.allocator, stage->memory, 0, data.size);

	auto copyToDevice = [this, stage, layer, level, x, y, w, h] (VulkanCommandBuffer& commands) {
		transitionImageLayout(commands.cmdbuffer, textureImage, VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, level, layer, 1);
		copyBufferToImage(commands.cmdbuffer, stage->buffer, textureImage, layer, level, x, y, w, h, nullptr);
		transitionImageLayout(commands.cmdbuffer, textureImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, level, layer, 1);

		mStagePool.releaseStage(stage, commands);
	};

	if (mContext.currentCommands)
	{
		copyToDevice(*mContext.currentCommands);
	}
	else
	{
		acquireWorkCommandBuffer(mContext);
		copyToDevice(mContext.work);
		flushWorkCommandBuffer(mContext);
	}
}

void VulkanTexture::updateCubeImage(const PixelBufferDescriptor& data,
        const FaceOffsets& faceOffsets, int miplevel) {
    assert(this->target == SamplerType::SAMPLER_CUBEMAP);
    const bool reshape = getBytesPerPixel(format) == 3;
    const void* cpuData = data.buffer;
    const uint32_t numSrcBytes = (uint32_t) data.size;
    const uint32_t numDstBytes = reshape ? (4 * numSrcBytes / 3) : numSrcBytes;

    // Create and populate the staging buffer.
    VulkanStage const* stage = mStagePool.acquireStage(numDstBytes);
    void* mapped;
    vmaMapMemory(mContext.allocator, stage->memory, &mapped);
    if (reshape) {
        DataReshaper::reshape<uint8_t, 3, 4>(mapped, cpuData, numSrcBytes);
    } else {
        memcpy(mapped, cpuData, numSrcBytes);
    }
    vmaUnmapMemory(mContext.allocator, stage->memory);
    vmaFlushAllocation(mContext.allocator, stage->memory, 0, numDstBytes);

    // Create a copy-to-device functor.
    auto copyToDevice = [this, faceOffsets, stage, miplevel] (VulkanCommandBuffer& commands) {
        uint32_t width = std::max(1u, this->width >> miplevel);
        uint32_t height = std::max(1u, this->height >> miplevel);
        transitionImageLayout(commands.cmdbuffer, textureImage, VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, miplevel, 0, 6);
        copyBufferToImage(commands.cmdbuffer, stage->buffer, textureImage, 0, miplevel, 0, 0, width, height, &faceOffsets);
        transitionImageLayout(commands.cmdbuffer, textureImage,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, miplevel, 0, 6);

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

void VulkanTexture::copyTexture(
	int dst_layer, int dst_level,
	const backend::Offset3D& dst_offset,
	const backend::Offset3D& dst_extent,
	VulkanTexture* src,
	int src_layer, int src_level,
	const backend::Offset3D& src_offset,
	const backend::Offset3D& src_extent,
	backend::SamplerMagFilter blit_filter)
{
	VulkanCommandBuffer* commands = nullptr;

	if (!mContext.currentCommands)
	{
		acquireWorkCommandBuffer(mContext);
		commands = &mContext.work;
	}
	else
	{
		commands = mContext.currentCommands;
	}

	// do blit
	{
		transitionImageLayout(commands->cmdbuffer, src->textureImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, src_level, src_layer, 1);
		transitionImageLayout(commands->cmdbuffer, this->textureImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dst_level, dst_layer, 1);

		VkImageBlit region = { };
		region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.srcSubresource.mipLevel = src_level;
		region.srcSubresource.baseArrayLayer = src_layer;
		region.srcSubresource.layerCount = 1;
		region.srcOffsets[0] = { src_offset.x, src_offset.y, src_offset.z };
		region.srcOffsets[1] = { src_offset.x + src_extent.x, src_offset.y + src_extent.y, src_offset.z + src_extent.z };
		region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.dstSubresource.mipLevel = dst_level;
		region.dstSubresource.baseArrayLayer = dst_layer;
		region.dstSubresource.layerCount = 1;
		region.dstOffsets[0] = { dst_offset.x, dst_offset.y, dst_offset.z };
		region.dstOffsets[1] = { dst_offset.x + dst_extent.x, dst_offset.y + dst_extent.y, dst_offset.z + dst_extent.z };

		vkCmdBlitImage(
			commands->cmdbuffer,
			src->textureImage,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			this->textureImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region,
			blit_filter == backend::SamplerMagFilter::LINEAR ? VK_FILTER_LINEAR : VK_FILTER_NEAREST);

		transitionImageLayout(commands->cmdbuffer, src->textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, src_level, src_layer, 1);
		transitionImageLayout(commands->cmdbuffer, this->textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, dst_level, dst_layer, 1);
	}

	if (!mContext.currentCommands)
	{
		flushWorkCommandBuffer(mContext);
	}
}

void VulkanTexture::copyTextureToMemory(
    int layer, int level,
    const Offset3D& offset,
    const Offset3D& extent,
    PixelBufferDescriptor& data)
{
    VulkanStage const* stage = mStagePool.acquireStage((uint32_t) data.size);

    auto copyToHost = [this, stage, layer, level, offset, extent](VulkanCommandBuffer& commands) {
        transitionImageLayout(commands.cmdbuffer, textureImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, level, layer, 1);

        VkBufferImageCopy region = { };
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.baseArrayLayer = layer;
        region.imageSubresource.layerCount = 1;
        region.imageSubresource.mipLevel = level;
        region.imageOffset = { offset.x, offset.y, offset.z };
        region.imageExtent = { (uint32_t) extent.x, (uint32_t) extent.y, (uint32_t) extent.z };
        vkCmdCopyImageToBuffer(commands.cmdbuffer, textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stage->buffer, 1, &region);

        transitionImageLayout(commands.cmdbuffer, textureImage,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, level, layer, 1);
    };

    if (mContext.currentCommands)
    {
        copyToHost(*mContext.currentCommands);
    }
    else
    {
        acquireWorkCommandBuffer(mContext);
        copyToHost(mContext.work);
        flushWorkCommandBuffer(mContext);
    }

    waitForIdle(mContext);

    void* mapped;
    vmaMapMemory(mContext.allocator, stage->memory, &mapped);
    memcpy(data.buffer, mapped, data.size);
    vmaUnmapMemory(mContext.allocator, stage->memory);

    mStagePool.releaseStage(stage);
}

void VulkanTexture::generateMipmaps() {
	int mipLevelCount = (int) floor(log2(float(std::max(this->width, this->height)))) + 1;
	int layerCount = (target == SamplerType::SAMPLER_CUBEMAP ? 6 : 1);

	auto blit = [this, mipLevelCount, layerCount] (VulkanCommandBuffer& commands) {
		transitionImageLayout(commands.cmdbuffer, textureImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 0, 0, layerCount);

		for (int i = 1; i < mipLevelCount; ++i) {
			transitionImageLayout(commands.cmdbuffer, textureImage, VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, i, 0, layerCount);

			VkImageBlit region = { };
			region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.srcSubresource.mipLevel = i - 1;
			region.srcSubresource.layerCount = layerCount;
			region.srcOffsets[1].x = std::max(this->width >> (i - 1), 1u);
			region.srcOffsets[1].y = std::max(this->height >> (i - 1), 1u);
			region.srcOffsets[1].z = 1;
			region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.dstSubresource.mipLevel = i;
			region.dstSubresource.layerCount = layerCount;
			region.dstOffsets[1].x = std::max(this->width >> i, 1u);
			region.dstOffsets[1].y = std::max(this->height >> i, 1u);
			region.dstOffsets[1].z = 1;

			vkCmdBlitImage(
				commands.cmdbuffer,
				textureImage,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				textureImage,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&region,
				VK_FILTER_LINEAR);

			if (i == mipLevelCount - 1) {
				transitionImageLayout(commands.cmdbuffer, textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, i, 0, layerCount);
			} else {
				transitionImageLayout(commands.cmdbuffer, textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, i, 0, layerCount);
			}
			
			transitionImageLayout(commands.cmdbuffer, textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, i - 1, 0, layerCount);
		}
	};

	// If inside beginFrame / endFrame, use the swap context, otherwise use the work cmdbuffer.
	if (mContext.currentCommands) {
		blit(*mContext.currentCommands);
	} else {
		acquireWorkCommandBuffer(mContext);
		blit(mContext.work);
		flushWorkCommandBuffer(mContext);
	}
}

void VulkanTexture::transitionImageLayout(VkCommandBuffer cmd, VkImage image,
        VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t baseLevel, uint32_t baseLayer, uint32_t layerCount) {
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = baseLevel;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = baseLayer;
    barrier.subresourceRange.layerCount = layerCount;
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;
    switch (newLayout) {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        default:
           PANIC_POSTCONDITION("Unsupported layout transition.");
    }
    vkCmdPipelineBarrier(cmd, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1,
            &barrier);
}

void VulkanTexture::transitionImageLayout(
	VkCommandBuffer cmd,
	VkImage image,
	VkPipelineStageFlags src_stage,
	VkPipelineStageFlags dst_stage,
	const VkImageSubresourceRange& subresource_range,
	VkImageLayout old_image_layout,
	VkImageLayout new_image_layout,
	VkAccessFlagBits src_access_mask)
{
	VkImageMemoryBarrier barrier_info = { };
	barrier_info.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier_info.pNext = NULL;
	barrier_info.srcAccessMask = src_access_mask;
	barrier_info.dstAccessMask = 0;
	barrier_info.oldLayout = old_image_layout;
	barrier_info.newLayout = new_image_layout;
	barrier_info.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier_info.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier_info.image = image;
	barrier_info.subresourceRange = subresource_range;

	switch (new_image_layout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		barrier_info.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		barrier_info.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		barrier_info.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		barrier_info.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		barrier_info.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		barrier_info.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		break;

	default:
		barrier_info.dstAccessMask = 0;
		break;
	}

	vkCmdPipelineBarrier(cmd, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier_info);
}

void VulkanTexture::copyBufferToImage(
	VkCommandBuffer cmd,
	VkBuffer buffer, VkImage image,
	int layer, int level,
	int x, int y,
	int w, int h,
	FaceOffsets const* faceOffsets)
{
	VkOffset3D offset = { x, y, 0 };
    VkExtent3D extent = { (uint32_t) w, (uint32_t) h, 1 };
    if (target == SamplerType::SAMPLER_CUBEMAP && faceOffsets)
	{
        VkBufferImageCopy regions[6] = { { } };
        for (size_t face = 0; face < 6; ++face)
		{
            auto& region = regions[face];
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.baseArrayLayer = (uint32_t) face;
            region.imageSubresource.layerCount = 1;
            region.imageSubresource.mipLevel = level;
			region.imageOffset = offset;
            region.imageExtent = extent;
            region.bufferOffset = faceOffsets->offsets[face];
        }
        vkCmdCopyBufferToImage(cmd, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6, regions);
    }
	else
	{
		VkBufferImageCopy region = { };
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.baseArrayLayer = layer;
		region.imageSubresource.layerCount = 1;
		region.imageSubresource.mipLevel = level;
		region.imageOffset = offset;
		region.imageExtent = extent;
		vkCmdCopyBufferToImage(cmd, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	}
}

void VulkanRenderPrimitive::setPrimitiveType(backend::PrimitiveType pt) {
    this->type = pt;
    switch (pt) {
        case backend::PrimitiveType::NONE:
        case backend::PrimitiveType::POINTS:
            primitiveTopology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            break;
        case backend::PrimitiveType::LINES:
            primitiveTopology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            break;
        case backend::PrimitiveType::TRIANGLES:
            primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            break;
    }
}

void VulkanRenderPrimitive::setBuffers(VulkanVertexBuffer* vertexBuffer,
        VulkanIndexBuffer* indexBuffer, uint32_t enabledAttributes) {
    this->vertexBuffer = vertexBuffer;
    this->indexBuffer = indexBuffer;
    const size_t nattrs = vertexBuffer->attributes.size();

    // These vectors are passed to vkCmdBindVertexBuffers at every draw call. This binds the
    // VkBuffer objects, but does not describe the structure of a vertex.
    buffers.clear();
    buffers.reserve(nattrs);
    offsets.clear();
    offsets.reserve(nattrs);

    // The following fixed-size arrays are consumed by VulkanBinder. They describe the vertex
    // structure, but do not specify the actual buffer objects to bind.
    memset(varray.attributes, 0, sizeof(varray.attributes));
    memset(varray.buffers, 0, sizeof(varray.buffers));

    // For each enabled attribute, append to each of the above lists. Note that a single VkBuffer
    // handle might be appended more than once, which is perfectly fine.
    uint32_t bufferIndex = 0;
    for (uint32_t attribIndex = 0; attribIndex < nattrs; attribIndex++) {
        if (!(enabledAttributes & (1U << attribIndex))) {
            continue;
        }
        const Attribute& attrib = vertexBuffer->attributes[attribIndex];
        buffers.push_back(vertexBuffer->buffers[attrib.buffer]->getGpuBuffer());
        offsets.push_back(attrib.offset);
        varray.attributes[bufferIndex] = {
            attribIndex, // matches the GLSL layout specifier
            bufferIndex,  // matches the position within vkCmdBindVertexBuffers
            getVkFormat(attrib.type, attrib.flags & Attribute::FLAG_NORMALIZED),
            0
        };
        varray.buffers[bufferIndex] = {
            bufferIndex,
            attrib.stride,
            VK_VERTEX_INPUT_RATE_VERTEX
        };
        bufferIndex++;
    }
}

} // namespace filament
} // namespace backend
