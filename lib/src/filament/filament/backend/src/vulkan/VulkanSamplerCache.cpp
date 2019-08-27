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

#include "vulkan/VulkanSamplerCache.h"

#include <utils/Panic.h>

namespace filament {
namespace backend {

constexpr inline VkSamplerAddressMode getWrapMode(SamplerWrapMode mode) noexcept {
    switch (mode) {
        case SamplerWrapMode::REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case SamplerWrapMode::CLAMP_TO_EDGE:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case SamplerWrapMode::MIRRORED_REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		default:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    }
}

constexpr inline VkFilter getFilter(SamplerMinFilter filter) noexcept {
    switch (filter) {
        case SamplerMinFilter::NEAREST:
            return VK_FILTER_NEAREST;
        case SamplerMinFilter::LINEAR:
            return VK_FILTER_LINEAR;
        case SamplerMinFilter::NEAREST_MIPMAP_NEAREST:
            return VK_FILTER_NEAREST;
        case SamplerMinFilter::LINEAR_MIPMAP_NEAREST:
            return VK_FILTER_LINEAR;
        case SamplerMinFilter::NEAREST_MIPMAP_LINEAR:
            return VK_FILTER_NEAREST;
        case SamplerMinFilter::LINEAR_MIPMAP_LINEAR:
            return VK_FILTER_LINEAR;
		default:
			return VK_FILTER_NEAREST;
    }
}

constexpr inline VkFilter getFilter(SamplerMagFilter filter) noexcept {
    switch (filter) {
        case SamplerMagFilter::NEAREST:
            return VK_FILTER_NEAREST;
        case SamplerMagFilter::LINEAR:
            return VK_FILTER_LINEAR;
		default:
			return VK_FILTER_NEAREST;
    }
}

constexpr inline VkSamplerMipmapMode getMipmapMode(SamplerMinFilter filter) noexcept {
    switch (filter) {
        case SamplerMinFilter::NEAREST:
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case SamplerMinFilter::LINEAR:
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case SamplerMinFilter::NEAREST_MIPMAP_NEAREST:
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case SamplerMinFilter::LINEAR_MIPMAP_NEAREST:
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case SamplerMinFilter::NEAREST_MIPMAP_LINEAR:
            return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        case SamplerMinFilter::LINEAR_MIPMAP_LINEAR:
            return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		default:
			return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    }
}

constexpr inline float getMaxLod(SamplerMinFilter filter) noexcept {
    switch (filter) {
        case SamplerMinFilter::NEAREST:
        case SamplerMinFilter::LINEAR:
            // The Vulkan spec recommends a max LOD of 0.25 to "disable" mipmapping.
            // See "Mapping of OpenGL to Vulkan filter modes" in the VK Spec.
            return 0.25f;
        case SamplerMinFilter::NEAREST_MIPMAP_NEAREST:
        case SamplerMinFilter::LINEAR_MIPMAP_NEAREST:
        case SamplerMinFilter::NEAREST_MIPMAP_LINEAR:
        case SamplerMinFilter::LINEAR_MIPMAP_LINEAR:
            // Assuming our maximum texture size is 4k, we'll never need more than 12 miplevels.
            return 12.0f;
		default:
			return 0.25f;
    }
}

constexpr inline VkBool32 getCompareEnable(SamplerCompareMode mode) noexcept {
    return mode == SamplerCompareMode::NONE ? VK_FALSE : VK_TRUE;
}

VulkanSamplerCache::VulkanSamplerCache(VulkanContext& context) : mContext(context) {}

VkSampler VulkanSamplerCache::getSampler(backend::SamplerParams params) noexcept {
    auto iter = mCache.find(params.u);
    if (UTILS_LIKELY(iter != mCache.end())) {
        return iter->second;
    }
	VkSamplerCreateInfo samplerInfo { };
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = getFilter(params.filterMag);
	samplerInfo.minFilter = getFilter(params.filterMin);
	samplerInfo.addressModeU = getWrapMode(params.wrapS);
	samplerInfo.addressModeV = getWrapMode(params.wrapT);
	samplerInfo.addressModeW = getWrapMode(params.wrapR);
	samplerInfo.anisotropyEnable = params.anisotropyLog2 == 0 ? 0u : 1u;
	samplerInfo.maxAnisotropy = (float)(1u << params.anisotropyLog2);
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = getCompareEnable(params.compareMode);
	samplerInfo.compareOp = getCompareOp(params.compareFunc);
	samplerInfo.mipmapMode = getMipmapMode(params.filterMin);
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = getMaxLod(params.filterMin);

    VkSampler sampler;
    VkResult error = vkCreateSampler(mContext.device, &samplerInfo, VKALLOC, &sampler);
    ASSERT_POSTCONDITION(!error, "Unable to create sampler.");
    mCache.insert({params.u, sampler});
    return sampler;
}

void VulkanSamplerCache::reset() noexcept {
    for (auto pair : mCache) {
        vkDestroySampler(mContext.device, pair.second, VKALLOC);
    }
    mCache.clear();
}

} // namespace filament
} // namespace backend
