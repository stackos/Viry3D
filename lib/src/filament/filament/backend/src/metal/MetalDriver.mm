/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include "private/backend/CommandStream.h"
#include "CommandStreamDispatcher.h"
#include "MetalDriver.h"
#include "MetalDriverFactory.h"

#include <CoreVideo/CVMetalTexture.h>
#include <CoreVideo/CVPixelBuffer.h>
#include <Metal/Metal.h>
#include <QuartzCore/QuartzCore.h>

#include <utils/Log.h>
#include <utils/Panic.h>

namespace filament {
namespace backend {

Driver* MetalDriverFactory::create(MetalPlatform* const platform) {
    return metal::MetalDriver::create(platform);
}

namespace metal {

UTILS_NOINLINE
Driver* MetalDriver::create(MetalPlatform* const platform) {
    assert(platform);
    return new MetalDriver(platform);
}

MetalDriver::MetalDriver(backend::MetalPlatform* platform) noexcept:
	DriverBase(new ConcreteDispatcher<MetalDriver>()),
    mPlatform(*platform) {

}

MetalDriver::~MetalDriver() noexcept {

}

void MetalDriver::beginFrame(int64_t monotonic_clock_ns, uint32_t frameId) {

}

void MetalDriver::setPresentationTime(int64_t monotonic_clock_ns) {

}

void MetalDriver::endFrame(uint32_t frameId) {

}

void MetalDriver::flush(int dummy) {

}

void MetalDriver::createVertexBufferR(Handle<HwVertexBuffer> vbh, uint8_t bufferCount,
        uint8_t attributeCount, uint32_t vertexCount, AttributeArray attributes,
        BufferUsage usage) {

}

void MetalDriver::createIndexBufferR(Handle<HwIndexBuffer> ibh, ElementType elementType,
        uint32_t indexCount, BufferUsage usage) {

}

void MetalDriver::createTextureR(Handle<HwTexture> th, SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t width, uint32_t height,
        uint32_t depth, TextureUsage usage) {

}

void MetalDriver::createSamplerGroupR(Handle<HwSamplerGroup> sbh, size_t size) {

}

void MetalDriver::createUniformBufferR(Handle<HwUniformBuffer> ubh, size_t size,
        BufferUsage usage) {

}

void MetalDriver::createRenderPrimitiveR(Handle<HwRenderPrimitive> rph, int dummy) {

}

void MetalDriver::createProgramR(Handle<HwProgram> rph, Program&& program) {

}

void MetalDriver::createDefaultRenderTargetR(Handle<HwRenderTarget> rth, int dummy) {

}

void MetalDriver::createRenderTargetR(Handle<HwRenderTarget> rth,
        TargetBufferFlags targetBufferFlags, uint32_t width, uint32_t height,
        uint8_t samples, TargetBufferInfo color,
        TargetBufferInfo depth, TargetBufferInfo stencil) {

}

void MetalDriver::createFenceR(Handle<HwFence> fh, int dummy) {

}

void MetalDriver::createSwapChainR(Handle<HwSwapChain> sch, void* nativeWindow, uint64_t flags) {

}

void MetalDriver::createStreamFromTextureIdR(Handle<HwStream>, intptr_t externalTextureId,
        uint32_t width, uint32_t height) {

}

Handle<HwVertexBuffer> MetalDriver::createVertexBufferS() noexcept {
    return { };
}

Handle<HwIndexBuffer> MetalDriver::createIndexBufferS() noexcept {
    return { };
}

Handle<HwTexture> MetalDriver::createTextureS() noexcept {
    return { };
}

Handle<HwSamplerGroup> MetalDriver::createSamplerGroupS() noexcept {
    return { };
}

Handle<HwUniformBuffer> MetalDriver::createUniformBufferS() noexcept {
    return { };
}

Handle<HwRenderPrimitive> MetalDriver::createRenderPrimitiveS() noexcept {
    return { };
}

Handle<HwProgram> MetalDriver::createProgramS() noexcept {
    return { };
}

Handle<HwRenderTarget> MetalDriver::createDefaultRenderTargetS() noexcept {
    return { };
}

Handle<HwRenderTarget> MetalDriver::createRenderTargetS() noexcept {
    return { };
}

Handle<HwFence> MetalDriver::createFenceS() noexcept {
    return { };
}

Handle<HwSwapChain> MetalDriver::createSwapChainS() noexcept {
    return { };
}

Handle<HwStream> MetalDriver::createStreamFromTextureIdS() noexcept {
    return { };
}

void MetalDriver::destroyVertexBuffer(Handle<HwVertexBuffer> vbh) {
    
}

void MetalDriver::destroyIndexBuffer(Handle<HwIndexBuffer> ibh) {
    
}

void MetalDriver::destroyRenderPrimitive(Handle<HwRenderPrimitive> rph) {
    
}

void MetalDriver::destroyProgram(Handle<HwProgram> ph) {
    
}

void MetalDriver::destroySamplerGroup(Handle<HwSamplerGroup> sbh) {

}

void MetalDriver::destroyUniformBuffer(Handle<HwUniformBuffer> ubh) {

}

void MetalDriver::destroyTexture(Handle<HwTexture> th) {

}

void MetalDriver::destroyRenderTarget(Handle<HwRenderTarget> rth) {

}

void MetalDriver::destroySwapChain(Handle<HwSwapChain> sch) {

}

void MetalDriver::destroyStream(Handle<HwStream> sh) {

}

void MetalDriver::terminate() {

}

ShaderModel MetalDriver::getShaderModel() const noexcept {
#if defined(IOS)
    return ShaderModel::GL_ES_30;
#else
    return ShaderModel::GL_CORE_41;
#endif
}

Handle<HwStream> MetalDriver::createStream(void* stream) {
    return { };
}

void MetalDriver::setStreamDimensions(Handle<HwStream> stream, uint32_t width,
        uint32_t height) {

}

int64_t MetalDriver::getStreamTimestamp(Handle<HwStream> stream) {
    return 0;
}

void MetalDriver::updateStreams(backend::DriverApi* driver) {

}

void MetalDriver::destroyFence(Handle<HwFence> fh) {

}

FenceStatus MetalDriver::wait(Handle<HwFence> fh, uint64_t timeout) {
    return FenceStatus::ERROR;
}

bool MetalDriver::isTextureFormatSupported(TextureFormat format) {
    return false;
}

bool MetalDriver::isRenderTargetFormatSupported(TextureFormat format) {
    return false;
}

bool MetalDriver::isFrameTimeSupported() {
	return false;
}

void MetalDriver::updateVertexBuffer(Handle<HwVertexBuffer> vbh, size_t index,
        BufferDescriptor&& data, uint32_t byteOffset) {

}

void MetalDriver::updateIndexBuffer(Handle<HwIndexBuffer> ibh, BufferDescriptor&& data,
        uint32_t byteOffset) {

}

void MetalDriver::update2DImage(Handle<HwTexture> th, uint32_t level, uint32_t xoffset,
        uint32_t yoffset, uint32_t width, uint32_t height, PixelBufferDescriptor&& data) {

}

void MetalDriver::updateCubeImage(Handle<HwTexture> th, uint32_t level,
        PixelBufferDescriptor&& data, FaceOffsets faceOffsets) {

}

void MetalDriver::setupExternalImage(void* image) {

}

void MetalDriver::cancelExternalImage(void* image) {

}

void MetalDriver::setExternalImage(Handle<HwTexture> th, void* image) {

}

void MetalDriver::setExternalStream(Handle<HwTexture> th, Handle<HwStream> sh) {

}

void MetalDriver::generateMipmaps(Handle<HwTexture> th) {

}

bool MetalDriver::canGenerateMipmaps() {
    return true;
}

void MetalDriver::loadUniformBuffer(Handle<HwUniformBuffer> ubh,
        BufferDescriptor&& data) {

}

void MetalDriver::updateSamplerGroup(Handle<HwSamplerGroup> sbh,
        SamplerGroup&& samplerGroup) {

}

void MetalDriver::beginRenderPass(Handle<HwRenderTarget> rth,
        const RenderPassParams& params) {
    
}

void MetalDriver::endRenderPass(int dummy) {

}

void MetalDriver::discardSubRenderTargetBuffers(Handle<HwRenderTarget> rth,
        TargetBufferFlags targetBufferFlags, uint32_t left, uint32_t bottom, uint32_t width,
        uint32_t height) {

}

void MetalDriver::setRenderPrimitiveBuffer(Handle<HwRenderPrimitive> rph,
        Handle<HwVertexBuffer> vbh, Handle<HwIndexBuffer> ibh, uint32_t enabledAttributes) {

}

void MetalDriver::setRenderPrimitiveRange(Handle<HwRenderPrimitive> rph,
        PrimitiveType pt, uint32_t offset, uint32_t minIndex, uint32_t maxIndex,
        uint32_t count) {

}

void MetalDriver::setViewportScissor(int32_t left, int32_t bottom, uint32_t width,
        uint32_t height) {

}

void MetalDriver::makeCurrent(Handle<HwSwapChain> schDraw, Handle<HwSwapChain> schRead) {

}

void MetalDriver::commit(Handle<HwSwapChain> sch) {

}

void MetalDriver::bindUniformBuffer(size_t index, Handle<HwUniformBuffer> ubh) {

}

void MetalDriver::bindUniformBufferRange(size_t index, Handle<HwUniformBuffer> ubh,
        size_t offset, size_t size) {

}

void MetalDriver::bindSamplers(size_t index, Handle<HwSamplerGroup> sbh) {

}

void MetalDriver::insertEventMarker(const char* string, size_t len) {

}

void MetalDriver::pushGroupMarker(const char* string, size_t len) {

}

void MetalDriver::popGroupMarker(int dummy) {

}

void MetalDriver::readPixels(Handle<HwRenderTarget> src, uint32_t x, uint32_t y, uint32_t width,
        uint32_t height, PixelBufferDescriptor&& data) {

}

void MetalDriver::readStreamPixels(Handle<HwStream> sh, uint32_t x, uint32_t y, uint32_t width,
        uint32_t height, PixelBufferDescriptor&& data) {

}

void MetalDriver::blit(TargetBufferFlags buffers,
        Handle<HwRenderTarget> dst, backend::Viewport dstRect,
        Handle<HwRenderTarget> src, backend::Viewport srcRect,
        SamplerMagFilter filter) {
    
}

void MetalDriver::draw(backend::PipelineState ps, Handle<HwRenderPrimitive> rph) {
    
}

} // namespace metal
} // namespace backend
} // namespace filament
