/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "private/backend/Driver.h"
#include "private/backend/CommandStream.h"

#include "DriverBase.h"

#include <math/half.h>
#include <math/quat.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <backend/BufferDescriptor.h>
#include <backend/PixelBufferDescriptor.h>

using namespace utils;

namespace filament {
namespace backend {

DriverBase::DriverBase(Dispatcher* dispatcher) noexcept
        : mDispatcher(dispatcher) {
}

DriverBase::~DriverBase() noexcept {
    delete mDispatcher;
}

void DriverBase::purge() noexcept {
    std::vector<BufferDescriptor> buffersToPurge;
    std::unique_lock<std::mutex> lock(mPurgeLock);
    std::swap(buffersToPurge, mBufferToPurge);
    lock.unlock(); // don't remove this, it ensures mBufferToPurge is destroyed without lock held
}

void DriverBase::scheduleDestroySlow(BufferDescriptor&& buffer) noexcept {
    std::lock_guard<std::mutex> lock(mPurgeLock);
    mBufferToPurge.push_back(std::move(buffer));
}

// ------------------------------------------------------------------------------------------------

Driver::~Driver() noexcept = default;

size_t Driver::getElementTypeSize(ElementType type) noexcept {
    switch (type) {
        case ElementType::BYTE:     return sizeof(int8_t);
        case ElementType::BYTE2:    return sizeof(math::byte2);
        case ElementType::BYTE3:    return sizeof(math::byte3);
        case ElementType::BYTE4:    return sizeof(math::byte4);
        case ElementType::UBYTE:    return sizeof(uint8_t);
        case ElementType::UBYTE2:   return sizeof(math::ubyte2);
        case ElementType::UBYTE3:   return sizeof(math::ubyte3);
        case ElementType::UBYTE4:   return sizeof(math::ubyte4);
        case ElementType::SHORT:    return sizeof(int16_t);
        case ElementType::SHORT2:   return sizeof(math::short2);
        case ElementType::SHORT3:   return sizeof(math::short3);
        case ElementType::SHORT4:   return sizeof(math::short4);
        case ElementType::USHORT:   return sizeof(uint16_t);
        case ElementType::USHORT2:  return sizeof(math::ushort2);
        case ElementType::USHORT3:  return sizeof(math::ushort3);
        case ElementType::USHORT4:  return sizeof(math::ushort4);
        case ElementType::INT:      return sizeof(int32_t);
        case ElementType::UINT:     return sizeof(uint32_t);
        case ElementType::FLOAT:    return sizeof(float);
        case ElementType::FLOAT2:   return sizeof(math::float2);
        case ElementType::FLOAT3:   return sizeof(math::float3);
        case ElementType::FLOAT4:   return sizeof(math::float4);
        case ElementType::HALF:     return sizeof(math::half);
        case ElementType::HALF2:    return sizeof(math::half2);
        case ElementType::HALF3:    return sizeof(math::half3);
        case ElementType::HALF4:    return sizeof(math::half4);
		default:					return 0;
    }
}

size_t getTextureFormatSize(TextureFormat format) noexcept {
	switch (format) {
		// 8-bits per element
	case TextureFormat::R8:
	case TextureFormat::R8_SNORM:
	case TextureFormat::R8UI:
	case TextureFormat::R8I:
	case TextureFormat::STENCIL8:
		return 1;

		// 16-bits per element
	case TextureFormat::R16F:
	case TextureFormat::R16UI:
	case TextureFormat::R16I:
	case TextureFormat::RG8:
	case TextureFormat::RG8_SNORM:
	case TextureFormat::RG8UI:
	case TextureFormat::RG8I:
	case TextureFormat::RGB565:
	case TextureFormat::RGB5_A1:
	case TextureFormat::RGBA4:
	case TextureFormat::DEPTH16:
		return 2;

		// 24-bits per element
	case TextureFormat::RGB8:
	case TextureFormat::SRGB8:
	case TextureFormat::RGB8_SNORM:
	case TextureFormat::RGB8UI:
	case TextureFormat::RGB8I:
	case TextureFormat::DEPTH24:
		return 3;

		// 32-bits per element
	case TextureFormat::R32F:
	case TextureFormat::R32UI:
	case TextureFormat::R32I:
	case TextureFormat::RG16F:
	case TextureFormat::RG16UI:
	case TextureFormat::RG16I:
	case TextureFormat::R11F_G11F_B10F:
	case TextureFormat::RGB9_E5:
	case TextureFormat::RGBA8:
	case TextureFormat::SRGB8_A8:
	case TextureFormat::RGBA8_SNORM:
	case TextureFormat::RGB10_A2:
	case TextureFormat::RGBA8UI:
	case TextureFormat::RGBA8I:
	case TextureFormat::DEPTH32F:
	case TextureFormat::DEPTH24_STENCIL8:
	case TextureFormat::DEPTH32F_STENCIL8:
		return 4;

		// 48-bits per element
	case TextureFormat::RGB16F:
	case TextureFormat::RGB16UI:
	case TextureFormat::RGB16I:
		return 6;

		// 64-bits per element
	case TextureFormat::RG32F:
	case TextureFormat::RG32UI:
	case TextureFormat::RG32I:
	case TextureFormat::RGBA16F:
	case TextureFormat::RGBA16UI:
	case TextureFormat::RGBA16I:
		return 8;

		// 96-bits per element
	case TextureFormat::RGB32F:
	case TextureFormat::RGB32UI:
	case TextureFormat::RGB32I:
		return 12;

		// 128-bits per element
	case TextureFormat::RGBA32F:
	case TextureFormat::RGBA32UI:
	case TextureFormat::RGBA32I:
		return 16;

	default:
		assert(false);
	}
	return 0;
}

} // namespace backend
} // namespace filament
