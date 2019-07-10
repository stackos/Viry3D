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

#include "vulkan/VulkanFboCache.h"

#include <utils/Panic.h>

namespace filament {
namespace backend {

bool VulkanFboCache::RenderPassEq::operator()(const RenderPassKey& k1,
        const RenderPassKey& k2) const {
    return
            k1.finalLayout == k2.finalLayout &&
            k1.colorFormat == k2.colorFormat &&
            k1.depthFormat == k2.depthFormat &&
            k1.flags.value == k2.flags.value;
}

bool VulkanFboCache::FboKeyEqualFn::operator()(const FboKey& k1, const FboKey& k2) const {
    static_assert(sizeof(FboKey::attachments) == 3 * sizeof(VkImageView), "Unexpected count.");
    return
            k1.renderPass == k2.renderPass &&
            k1.attachments[0] == k2.attachments[0] &&
            k1.attachments[1] == k2.attachments[1] &&
            k1.attachments[2] == k2.attachments[2];
}

VulkanFboCache::VulkanFboCache(VulkanContext& context) : mContext(context) {}

VulkanFboCache::~VulkanFboCache() {
    ASSERT_POSTCONDITION(mFramebufferCache.empty() && mRenderPassCache.empty(),
            "Please explicitly call reset() while the VkDevice is still alive.");
}

VkFramebuffer VulkanFboCache::getFramebuffer(FboKey config, uint32_t w, uint32_t h) noexcept {
    auto iter = mFramebufferCache.find(config);
    if (UTILS_LIKELY(iter != mFramebufferCache.end() && iter->second.handle != VK_NULL_HANDLE)) {
        iter->second.timestamp = mCurrentTime;
        return iter->second.handle;
    }
    uint32_t nAttachments = 0;
    for (auto attachment : config.attachments) {
        if (attachment) {
            nAttachments++;
        }
    }
	VkFramebufferCreateInfo info { };
	info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	info.renderPass = config.renderPass;
	info.width = w;
	info.height = h;
	info.layers = 1;
	info.attachmentCount = nAttachments;
	info.pAttachments = config.attachments;

    mRenderPassRefCount[info.renderPass]++;
    VkFramebuffer framebuffer;
    VkResult error = vkCreateFramebuffer(mContext.device, &info, VKALLOC, &framebuffer);
    ASSERT_POSTCONDITION(!error, "Unable to create framebuffer.");
    mFramebufferCache[config] = {framebuffer, mCurrentTime};
    return framebuffer;
}

VkRenderPass VulkanFboCache::getRenderPass(RenderPassKey config) noexcept {
	auto iter = mRenderPassCache.find(config);
	if (UTILS_LIKELY(iter != mRenderPassCache.end() && iter->second.handle != VK_NULL_HANDLE)) {
		iter->second.timestamp = mCurrentTime;
		return iter->second.handle;
	}
	const bool hasColor = config.colorFormat != VK_FORMAT_UNDEFINED;
	const bool hasDepth = config.depthFormat != VK_FORMAT_UNDEFINED;
	const bool depthOnly = hasDepth && !hasColor;

	// The subpass specifies the layout to transition to at the START of the render pass.
	uint32_t numAttachments = 0;
	VkAttachmentReference colorAttachmentRef = {};
	if (hasColor) {
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachmentRef.attachment = numAttachments++;
	}
	VkAttachmentReference depthAttachmentRef = {};
	if (hasDepth) {
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachmentRef.attachment = numAttachments++;
	}
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = hasColor ? 1u : 0u;
	subpass.pColorAttachments = hasColor ? &colorAttachmentRef : nullptr;
	subpass.pDepthStencilAttachment = hasDepth ? &depthAttachmentRef : nullptr;

	VkAttachmentLoadOp color_load;
	VkAttachmentLoadOp depth_load;
	VkImageLayout color_initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkImageLayout depth_initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;

	if (config.flags.clear & TargetBufferFlags::COLOR)
	{
		color_load = VK_ATTACHMENT_LOAD_OP_CLEAR;
	}
	else
	{
		if (config.flags.discardStart & TargetBufferFlags::COLOR)
		{
			color_load = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		}
		else
		{
			color_load = VK_ATTACHMENT_LOAD_OP_LOAD;
			color_initial_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
	}

	if (config.flags.clear & TargetBufferFlags::DEPTH)
	{
		depth_load = VK_ATTACHMENT_LOAD_OP_CLEAR;
	}
	else
	{
		if (config.flags.discardStart & TargetBufferFlags::DEPTH)
		{
			depth_load = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		}
		else
		{
			depth_load = VK_ATTACHMENT_LOAD_OP_LOAD;
			depth_initial_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
	}

	// The attachment description specifies the layout to transition to at the END of the render pass.
    VkAttachmentDescription colorAttachment {
		0,
        config.colorFormat,
        VK_SAMPLE_COUNT_1_BIT,
		color_load,
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
		color_initial_layout,
        config.finalLayout
    };
    VkAttachmentDescription depthAttachment {
		0,
        config.depthFormat,
        VK_SAMPLE_COUNT_1_BIT,
		depth_load,
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
		depth_initial_layout,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
    };

    // We define dependencies only when the framebuffer local hint is applied.
    // NOTE: It's likely that VK_DEPENDENCY_BY_REGION_BIT and VK_ACCESS_COLOR_ATTACHMENT_READ do
    // not actually achieve anything since are neither defining multiple subpasses, nor reading back
    // from the framebuffer in the shader using subpassLoad().
    VkSubpassDependency dependencies[] = {{
        VK_SUBPASS_EXTERNAL,
        0,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_MEMORY_READ_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        config.flags.dependencies
    }, {
        0,
        VK_SUBPASS_EXTERNAL,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        VK_ACCESS_MEMORY_READ_BIT,
        config.flags.dependencies
    }};

    // Finally, create the VkRenderPass.
    VkAttachmentDescription attachments[2];
    VkRenderPassCreateInfo renderPassInfo {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		nullptr,
		0,
        0u,
        attachments,
		1,
		&subpass,
        config.flags.dependencies ? 2u : 0u,
        config.flags.dependencies ? dependencies : nullptr
    };
    if (hasColor) {
        attachments[renderPassInfo.attachmentCount++] = colorAttachment;
    }
    if (hasDepth) {
        attachments[renderPassInfo.attachmentCount++] = depthAttachment;
    }
    VkRenderPass renderPass;
    VkResult error = vkCreateRenderPass(mContext.device, &renderPassInfo, VKALLOC, &renderPass);
    ASSERT_POSTCONDITION(!error, "Unable to create render pass.");
    mRenderPassCache[config] = {renderPass, mCurrentTime};
    return renderPass;
}

void VulkanFboCache::reset() noexcept {
    for (auto pair : mFramebufferCache) {
        mRenderPassRefCount[pair.first.renderPass]--;
        vkDestroyFramebuffer(mContext.device, pair.second.handle, VKALLOC);
    }
    mFramebufferCache.clear();
    for (auto pair : mRenderPassCache) {
        vkDestroyRenderPass(mContext.device, pair.second.handle, VKALLOC);
    }
    mRenderPassCache.clear();
}

// Frees up old framebuffers and render passes, then nulls out their key.  Doesn't bother removing
// the actual map entry since it is fairly small.
void VulkanFboCache::gc() noexcept {
    mCurrentTime++;
    const uint32_t evictTime = mCurrentTime - TIME_BEFORE_EVICTION;
    for (auto iter = mFramebufferCache.begin(); iter != mFramebufferCache.end(); ++iter) {
        if (iter->second.timestamp < evictTime) {
            vkDestroyFramebuffer(mContext.device, iter->second.handle, VKALLOC);
            iter->second.handle = VK_NULL_HANDLE;
        }
    }
    for (auto iter = mRenderPassCache.begin(); iter != mRenderPassCache.end(); ++iter) {
        VkRenderPass handle = iter->second.handle;
        if (iter->second.timestamp < evictTime && mRenderPassRefCount[handle] == 0) {
            vkDestroyRenderPass(mContext.device, handle, VKALLOC);
            iter->second.handle = VK_NULL_HANDLE;
        }
    }
}

} // namespace filament
} // namespace backend
