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

#include "D3D11Driver.h"
#include "D3D11DriverFactory.h"
#include "CommandStreamDispatcher.h"
#include "D3D11Context.h"
#include "D3D11Handles.h"
#include <utils/unwindows.h>

namespace filament
{
	namespace backend
	{
		Driver* D3D11DriverFactory::create(backend::D3D11Platform* platform)
		{
			return D3D11Driver::create(platform);
		}

		Driver* D3D11Driver::create(backend::D3D11Platform* platform)
		{
			assert(platform);
			return new D3D11Driver(platform);
		}

		D3D11Driver::D3D11Driver(backend::D3D11Platform* platform) noexcept:
			DriverBase(new ConcreteDispatcher<D3D11Driver>()),
			m_platform(*platform),
			m_context(new D3D11Context())
		{
		
		}

		D3D11Driver::~D3D11Driver() noexcept
		{
			delete m_context;
		}

		ShaderModel D3D11Driver::getShaderModel() const noexcept
		{
			return ShaderModel::GL_CORE_41;
		}

		void D3D11Driver::setPresentationTime(int64_t monotonic_clock_ns)
		{

		}

		void D3D11Driver::beginFrame(int64_t monotonic_clock_ns, uint32_t frame_id)
		{
			
		}

		void D3D11Driver::endFrame(uint32_t frame_id)
		{

		}

		void D3D11Driver::flush(int dummy)
		{

		}

		void D3D11Driver::createVertexBufferR(
			Handle<HwVertexBuffer> vbh,
			uint8_t buffer_count,
			uint8_t attribute_count,
			uint32_t vertex_count,
			AttributeArray attributes,
			BufferUsage usage)
		{
			construct_handle<D3D11VertexBuffer>(
				m_handle_map,
				vbh,
				m_context,
				buffer_count,
				attribute_count,
				vertex_count,
				attributes,
				usage);
		}

		void D3D11Driver::createIndexBufferR(
			Handle<HwIndexBuffer> ibh,
			ElementType element_type,
			uint32_t index_count,
			BufferUsage usage)
		{
			construct_handle<D3D11IndexBuffer>(
				m_handle_map,
				ibh,
				m_context,
				element_type,
				index_count,
				usage);
		}

		void D3D11Driver::createTextureR(
			Handle<HwTexture> th,
			SamplerType target,
			uint8_t levels,
			TextureFormat format,
			uint8_t samples,
			uint32_t width,
			uint32_t height,
			uint32_t depth,
			TextureUsage usage)
		{
			construct_handle<D3D11Texture>(m_handle_map, th, m_context, target, levels, format, samples, width, height, depth, usage);
		}

		void D3D11Driver::createSamplerGroupR(Handle<HwSamplerGroup> sgh, size_t size)
		{
			construct_handle<D3D11SamplerGroup>(m_handle_map, sgh, m_context, size);
		}

		void D3D11Driver::createUniformBufferR(Handle<HwUniformBuffer> ubh, size_t size, BufferUsage usage)
		{
			construct_handle<D3D11UniformBuffer>(m_handle_map, ubh, m_context, size, usage);
		}

		void D3D11Driver::createRenderPrimitiveR(Handle<HwRenderPrimitive> rph, int dummy)
		{
			construct_handle<D3D11RenderPrimitive>(m_handle_map, rph, m_context);
		}

		void D3D11Driver::createProgramR(Handle<HwProgram> ph, Program&& program)
		{
			construct_handle<D3D11Program>(m_handle_map, ph, m_context, std::move(program));
		}

		void D3D11Driver::createDefaultRenderTargetR(Handle<HwRenderTarget> rth, int dummy)
		{
			construct_handle<D3D11RenderTarget>(m_handle_map, rth, m_context);
		}

		void D3D11Driver::createRenderTargetR(
			Handle<HwRenderTarget> rth,
			TargetBufferFlags flags,
			uint32_t width,
			uint32_t height,
			uint8_t samples,
			TargetBufferInfo color,
			TargetBufferInfo depth,
			TargetBufferInfo stencil)
		{
			auto render_target = construct_handle<D3D11RenderTarget>(
				m_handle_map,
				rth,
				m_context,
				flags,
				width,
				height,
				samples,
				color,
				depth,
				stencil);

			assert(samples == 1);

			if (flags & TargetBufferFlags::COLOR)
			{
				auto color_texture = handle_cast<D3D11Texture>(m_handle_map, color.handle);

				D3D11_RENDER_TARGET_VIEW_DESC1 color_desc = { };
				color_desc.Format = m_context->GetTextureViewFormat(color_texture->format);
				color_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
				color_desc.Texture2D.MipSlice = color.level;

				HRESULT hr = m_context->device->CreateRenderTargetView1(
					color_texture->texture,
					&color_desc,
					&render_target->color_view);
				assert(SUCCEEDED(hr));
			}
			if ((flags & TargetBufferFlags::DEPTH) ||
				(flags & TargetBufferFlags::STENCIL))
			{
				D3D11Texture* texture = nullptr;
				UINT level = 0;

				if (flags & TargetBufferFlags::DEPTH)
				{
					texture = handle_cast<D3D11Texture>(m_handle_map, depth.handle);
					level = depth.level;
				}
				else if (flags & TargetBufferFlags::STENCIL)
				{
					texture = handle_cast<D3D11Texture>(m_handle_map, stencil.handle);
					level = stencil.level;
				}

				D3D11_DEPTH_STENCIL_VIEW_DESC depth_desc = { };
				depth_desc.Format = m_context->GetDepthViewFormat(texture->format);
				depth_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
				depth_desc.Texture2D.MipSlice = level;

				HRESULT hr = m_context->device->CreateDepthStencilView(
					texture->texture,
					&depth_desc,
					&render_target->depth_view);
				assert(SUCCEEDED(hr));
			}
		}

		void D3D11Driver::createFenceR(Handle<HwFence> fh, int dummy)
		{

		}

		void D3D11Driver::createSwapChainR(Handle<HwSwapChain> sch, void* native_window, uint64_t flags)
		{
			construct_handle<D3D11SwapChain>(m_handle_map, sch, m_context, native_window);
		}

		void D3D11Driver::createStreamFromTextureIdR(
			Handle<HwStream> stream,
			intptr_t externalTextureId,
			uint32_t width,
			uint32_t height)
		{

		}

		Handle<HwVertexBuffer> D3D11Driver::createVertexBufferS() noexcept
		{
			return alloc_handle<D3D11VertexBuffer, HwVertexBuffer>();
		}

		Handle<HwIndexBuffer> D3D11Driver::createIndexBufferS() noexcept
		{
			return alloc_handle<D3D11IndexBuffer, HwIndexBuffer>();
		}

		Handle<HwTexture> D3D11Driver::createTextureS() noexcept
		{
			return alloc_handle<D3D11Texture, HwTexture>();
		}

		Handle<HwSamplerGroup> D3D11Driver::createSamplerGroupS() noexcept
		{
			return alloc_handle<D3D11SamplerGroup, HwSamplerGroup>();
		}

		Handle<HwUniformBuffer> D3D11Driver::createUniformBufferS() noexcept
		{
			return alloc_handle<D3D11UniformBuffer, HwUniformBuffer>();
		}

		Handle<HwRenderPrimitive> D3D11Driver::createRenderPrimitiveS() noexcept
		{
			return alloc_handle<D3D11RenderPrimitive, HwRenderPrimitive>();
		}

		Handle<HwProgram> D3D11Driver::createProgramS() noexcept
		{
			return alloc_handle<D3D11Program, HwProgram>();
		}

		Handle<HwRenderTarget> D3D11Driver::createDefaultRenderTargetS() noexcept
		{
			return alloc_handle<D3D11RenderTarget, HwRenderTarget>();
		}

		Handle<HwRenderTarget> D3D11Driver::createRenderTargetS() noexcept
		{
			return alloc_handle<D3D11RenderTarget, HwRenderTarget>();
		}

		Handle<HwFence> D3D11Driver::createFenceS() noexcept
		{
			return Handle<HwFence>();
		}

		Handle<HwSwapChain> D3D11Driver::createSwapChainS() noexcept
		{
			return alloc_handle<D3D11SwapChain, HwSwapChain>();
		}

		Handle<HwStream> D3D11Driver::createStreamFromTextureIdS() noexcept
		{
			return Handle<HwStream>();
		}

		void D3D11Driver::destroyVertexBuffer(Handle<HwVertexBuffer> vbh)
		{
			destruct_handle<D3D11VertexBuffer>(m_handle_map, vbh);
		}

		void D3D11Driver::destroyIndexBuffer(Handle<HwIndexBuffer> ibh)
		{
			destruct_handle<D3D11IndexBuffer>(m_handle_map, ibh);
		}

		void D3D11Driver::destroyRenderPrimitive(Handle<HwRenderPrimitive> rph)
		{
			destruct_handle<D3D11RenderPrimitive>(m_handle_map, rph);
		}

		void D3D11Driver::destroyProgram(Handle<HwProgram> ph)
		{
			destruct_handle<D3D11Program>(m_handle_map, ph);
		}

		void D3D11Driver::destroySamplerGroup(Handle<HwSamplerGroup> sgh)
		{
			destruct_handle<D3D11SamplerGroup>(m_handle_map, sgh);
		}

		void D3D11Driver::destroyUniformBuffer(Handle<HwUniformBuffer> ubh)
		{
			destruct_handle<D3D11UniformBuffer>(m_handle_map, ubh);
		}

		void D3D11Driver::destroyTexture(Handle<HwTexture> th)
		{
			destruct_handle<D3D11Texture>(m_handle_map, th);
		}

		void D3D11Driver::destroyRenderTarget(Handle<HwRenderTarget> rth)
		{
			destruct_handle<D3D11RenderTarget>(m_handle_map, rth);
		}

		void D3D11Driver::destroySwapChain(Handle<HwSwapChain> sch)
		{
			destruct_handle<D3D11SwapChain>(m_handle_map, sch);
		}

		void D3D11Driver::destroyStream(Handle<HwStream> sh)
		{

		}

		void D3D11Driver::terminate()
		{

		}

		Handle<HwStream> D3D11Driver::createStream(void* stream)
		{
			return Handle<HwStream>();
		}

		void D3D11Driver::setStreamDimensions(Handle<HwStream> stream, uint32_t width, uint32_t height)
		{

		}

		int64_t D3D11Driver::getStreamTimestamp(Handle<HwStream> stream)
		{
			return 0;
		}

		void D3D11Driver::updateStreams(backend::DriverApi* driver)
		{

		}

		void D3D11Driver::destroyFence(Handle<HwFence> fh)
		{

		}

		FenceStatus D3D11Driver::wait(Handle<HwFence> fh, uint64_t timeout)
		{
			return FenceStatus::ERROR;
		}

		bool D3D11Driver::isTextureFormatSupported(TextureFormat format)
		{
			UINT support = 0;
			m_context->device->CheckFormatSupport(m_context->GetTextureFormat(format), &support);
			if (support & D3D11_FORMAT_SUPPORT_TEXTURE2D)
			{
				m_context->device->CheckFormatSupport(m_context->GetTextureViewFormat(format), &support);
				if (support & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE)
				{
					return true;
				}
			}
			return false;
		}

		bool D3D11Driver::isRenderTargetFormatSupported(TextureFormat format)
		{
			return this->isTextureFormatSupported(format);
		}

		bool D3D11Driver::isFrameTimeSupported()
		{
			return false;
		}

		void D3D11Driver::updateVertexBuffer(
			Handle<HwVertexBuffer> vbh,
			size_t index,
			BufferDescriptor&& data,
			uint32_t offset)
		{
			auto buffer = handle_cast<D3D11VertexBuffer>(m_handle_map, vbh);
			buffer->Update(m_context, index, data, offset);
			this->scheduleDestroy(std::move(data));
		}

		void D3D11Driver::updateIndexBuffer(
			Handle<HwIndexBuffer> ibh,
			BufferDescriptor&& data,
			uint32_t offset)
		{
			auto buffer = handle_cast<D3D11IndexBuffer>(m_handle_map, ibh);
			buffer->Update(m_context, data, offset);
			this->scheduleDestroy(std::move(data));
		}

		void D3D11Driver::updateTexture(
			Handle<HwTexture> th,
			int layer, int level,
			int x, int y,
			int w, int h,
			PixelBufferDescriptor&& data)
		{
			auto texture = handle_cast<D3D11Texture>(m_handle_map, th);
			texture->UpdateTexture(
				m_context,
				layer, level,
				x, y,
				w, h,
				data);
			this->scheduleDestroy(std::move(data));
		}

		void D3D11Driver::update2DImage(
			Handle<HwTexture> th,
			uint32_t level,
			uint32_t x, uint32_t y,
			uint32_t width, uint32_t height,
			PixelBufferDescriptor&& data)
		{
			auto texture = handle_cast<D3D11Texture>(m_handle_map, th);
			texture->UpdateTexture(
				m_context,
				0, level,
				x, y,
				width, height,
				data);
			this->scheduleDestroy(std::move(data));
		}

		void D3D11Driver::updateCubeImage(
			Handle<HwTexture> th,
			uint32_t level,
			PixelBufferDescriptor&& data,
			FaceOffsets face_offsets)
		{
			auto texture = handle_cast<D3D11Texture>(m_handle_map, th);
			for (int i = 0; i < 6; ++i)
			{
				auto buffer = static_cast<uint8_t*>(data.buffer) + face_offsets[i];
				if (data.type == PixelDataType::COMPRESSED)
				{
					texture->UpdateTexture(
						m_context,
						i, level,
						0, 0,
						texture->width >> level, texture->height >> level,
						PixelBufferDescriptor(buffer, data.size / 6, data.compressedFormat, data.imageSize, nullptr));
				}
				else
				{
					texture->UpdateTexture(
						m_context,
						i, level,
						0, 0,
						texture->width >> level, texture->height >> level,
						PixelBufferDescriptor(buffer, data.size / 6, data.format, data.type));
				}
			}
			this->scheduleDestroy(std::move(data));
		}

		void D3D11Driver::copyTexture(
			Handle<HwTexture> th_dst, int dst_layer, int dst_level,
			Offset3D dst_offset,
			Offset3D dst_extent,
			Handle<HwTexture> th_src, int src_layer, int src_level,
			Offset3D src_offset,
			Offset3D src_extent,
			SamplerMagFilter blit_filter)
		{
			auto dst = handle_cast<D3D11Texture>(m_handle_map, th_dst);
			auto src = handle_cast<D3D11Texture>(m_handle_map, th_src);
			dst->CopyTexture(
				m_context,
				dst_layer, dst_level,
				dst_offset, dst_extent,
				src,
				src_layer, src_level,
				src_offset, src_extent);
		}

		void D3D11Driver::copyTextureToMemory(
			Handle<HwTexture> th,
			int layer, int level,
			Offset3D offset,
			Offset3D extent,
			PixelBufferDescriptor&& data,
			std::function<void(const PixelBufferDescriptor&)> on_complete)
		{
			auto texture = handle_cast<D3D11Texture>(m_handle_map, th);
			texture->CopyTextureToMemory(
				m_context,
				layer, level,
				offset, extent,
				data);
			if (on_complete)
			{
				on_complete(data);
			}
			this->scheduleDestroy(std::move(data));
		}

		void D3D11Driver::setupExternalImage(void* image)
		{

		}

		void D3D11Driver::cancelExternalImage(void* image)
		{

		}

		void D3D11Driver::setExternalImage(Handle<HwTexture> th, void* image)
		{

		}

		void D3D11Driver::setExternalStream(Handle<HwTexture> th, Handle<HwStream> sh)
		{

		}

		void D3D11Driver::generateMipmaps(Handle<HwTexture> th)
		{
			auto texture = handle_cast<D3D11Texture>(m_handle_map, th);
			texture->GenerateMipmaps(m_context);
		}

		bool D3D11Driver::canGenerateMipmaps()
		{
			return true;
		}

		void D3D11Driver::loadUniformBuffer(Handle<HwUniformBuffer> ubh, BufferDescriptor&& data)
		{
			auto uniform_buffer = handle_cast<D3D11UniformBuffer>(m_handle_map, ubh);
			uniform_buffer->Load(m_context, data);
			this->scheduleDestroy(std::move(data));
		}

		void D3D11Driver::updateSamplerGroup(Handle<HwSamplerGroup> sgh, SamplerGroup&& sg)
		{
			auto sampler_group = handle_cast<D3D11SamplerGroup>(m_handle_map, sgh);
			sampler_group->Update(m_context, std::move(sg));
		}

		void D3D11Driver::beginRenderPass(
			Handle<HwRenderTarget> rth,
			const RenderPassParams& params)
		{
			auto render_target = handle_cast<D3D11RenderTarget>(m_handle_map, rth);
			m_context->current_render_target = render_target;
			m_context->current_render_pass_flags = params.flags;

			ID3D11RenderTargetView* color = nullptr;
			ID3D11DepthStencilView* depth = nullptr;
			uint32_t target_height = 0;

			// set render target
			if (render_target->default_render_target)
			{
				color = m_context->current_swap_chain->color_view;
				
				if (render_target->depth_view == nullptr)
				{
					DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = { };
					m_context->current_swap_chain->swap_chain->GetDesc1(&swap_chain_desc);

					render_target->CreateDepth(
						m_context,
						DXGI_FORMAT_D24_UNORM_S8_UINT,
						swap_chain_desc.Width,
						swap_chain_desc.Height);
					render_target->height = swap_chain_desc.Height;
				}

				depth = render_target->depth_view;
				target_height = render_target->height;
			}
			else
			{
				color = render_target->color_view;
				depth = render_target->depth_view;
				target_height = render_target->height;
			}

			if (color)
			{
				m_context->context->OMSetRenderTargets(1, &color, depth);
			}
			else
			{
				m_context->context->OMSetRenderTargets(0, nullptr, depth);
			}

			// set viewport
			D3D11_VIEWPORT viewport = CD3D11_VIEWPORT(
				(float) params.viewport.left,
				(float) (target_height - (params.viewport.bottom + params.viewport.height)),
				(float) params.viewport.width,
				(float) params.viewport.height
			);
			m_context->context->RSSetViewports(1, &viewport);

			// clear
			if (params.flags.clear & filament::backend::TargetBufferFlags::COLOR)
			{
				if (color)
				{
					m_context->context->ClearRenderTargetView(color, (float*) &params.clearColor);
				}
			}
			if (params.flags.clear & filament::backend::TargetBufferFlags::DEPTH ||
				params.flags.clear & filament::backend::TargetBufferFlags::STENCIL)
			{
				UINT flags = 0;
				if (params.flags.clear & filament::backend::TargetBufferFlags::DEPTH)
				{
					flags |= D3D11_CLEAR_DEPTH;
				}
				if (params.flags.clear & filament::backend::TargetBufferFlags::STENCIL)
				{
					flags |= D3D11_CLEAR_STENCIL;
				}

				if (depth)
				{
					m_context->context->ClearDepthStencilView(depth, flags, (float) params.clearDepth, params.clearStencil);
				}
			}
		}

		void D3D11Driver::endRenderPass(int dummy)
		{
			m_context->current_render_target = nullptr;
			m_context->current_render_pass_flags = { };

			for (size_t i = 0; i < m_context->uniform_buffer_bindings.size(); ++i)
			{
				m_context->uniform_buffer_bindings[i] = { };
			}

			for (size_t i = 0; i < m_context->sampler_group_binding.size(); ++i)
			{
				m_context->sampler_group_binding[i].sampler_group = SamplerGroupHandle();
			}

			ID3D11ShaderResourceView* null_views[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = { };
			m_context->context->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, null_views);
		}

		void D3D11Driver::discardSubRenderTargetBuffers(
			Handle<HwRenderTarget> rth,
			TargetBufferFlags targetBufferFlags,
			uint32_t left,
			uint32_t bottom,
			uint32_t width,
			uint32_t height)
		{

		}

		void D3D11Driver::setRenderPrimitiveBuffer(
			Handle<HwRenderPrimitive> rph,
			Handle<HwVertexBuffer> vbh,
			Handle<HwIndexBuffer> ibh,
			uint32_t enabled_attributes)
		{
			auto primitive = handle_cast<D3D11RenderPrimitive>(m_handle_map, rph);
			auto vertex_buffer = handle_cast<D3D11VertexBuffer>(m_handle_map, vbh);
			primitive->SetBuffer(m_context, vbh, ibh, enabled_attributes, vertex_buffer->vertexCount);
		}

		void D3D11Driver::setRenderPrimitiveRange(
			Handle<HwRenderPrimitive> rph,
			PrimitiveType pt,
			uint32_t offset,
			uint32_t min_index,
			uint32_t max_index,
			uint32_t count)
		{
			auto primitive = handle_cast<D3D11RenderPrimitive>(m_handle_map, rph);
			primitive->SetRange(m_context, pt, offset, min_index, max_index, count);
		}

		void D3D11Driver::setViewportScissor(
			int32_t left,
			int32_t bottom,
			uint32_t width,
			uint32_t height)
		{
			CD3D11_RECT rect(
				(LONG) left,
				(LONG) (m_context->current_render_target->height - (bottom + height)),
				(LONG) left + width,
				(LONG) bottom + height);
			m_context->context->RSSetScissorRects(1, &rect);
		}

		void D3D11Driver::makeCurrent(Handle<HwSwapChain> sch_draw, Handle<HwSwapChain> sch_read)
		{
			auto swap_chain = handle_cast<D3D11SwapChain>(m_handle_map, sch_draw);
			m_context->current_swap_chain = swap_chain;
		}

		void D3D11Driver::commit(Handle<HwSwapChain> sch)
		{
			auto swap_chain = handle_cast<D3D11SwapChain>(m_handle_map, sch);
			DXGI_PRESENT_PARAMETERS parameters = { };
			HRESULT hr = swap_chain->swap_chain->Present1(1, 0, &parameters);
			assert(SUCCEEDED(hr));
		}

		void D3D11Driver::bindUniformBuffer(size_t index, Handle<HwUniformBuffer> ubh)
		{
			auto uniform_buffer = handle_cast<D3D11UniformBuffer>(m_handle_map, ubh);

			assert(index < m_context->uniform_buffer_bindings.size());
			
			m_context->uniform_buffer_bindings[index].buffer = uniform_buffer->buffer;
			m_context->uniform_buffer_bindings[index].offset = 0;
			m_context->uniform_buffer_bindings[index].size = uniform_buffer->size;
		}

		void D3D11Driver::bindUniformBufferRange(
			size_t index,
			Handle<HwUniformBuffer> ubh,
			size_t offset,
			size_t size)
		{
			auto uniform_buffer = handle_cast<D3D11UniformBuffer>(m_handle_map, ubh);

			assert(index < m_context->uniform_buffer_bindings.size());
			assert(offset < uniform_buffer->size);
			assert(offset + size <= uniform_buffer->size);

			m_context->uniform_buffer_bindings[index].buffer = uniform_buffer->buffer;
			m_context->uniform_buffer_bindings[index].offset = offset;
			m_context->uniform_buffer_bindings[index].size = size;
		}

		void D3D11Driver::bindSamplers(size_t index, Handle<HwSamplerGroup> sgh)
		{
			auto sampler_group = handle_cast<D3D11SamplerGroup>(m_handle_map, sgh);

			assert(index < m_context->sampler_group_binding.size());

			m_context->sampler_group_binding[index].sampler_group = sgh;
		}

		void D3D11Driver::insertEventMarker(const char* string, size_t len)
		{

		}

		void D3D11Driver::pushGroupMarker(const char* string, size_t len)
		{

		}

		void D3D11Driver::popGroupMarker(int dummy)
		{

		}

		void D3D11Driver::readPixels(
			Handle<HwRenderTarget> src,
			uint32_t x,
			uint32_t y,
			uint32_t width,
			uint32_t height,
			PixelBufferDescriptor&& data)
		{

		}

		void D3D11Driver::readStreamPixels(
			Handle<HwStream> sh,
			uint32_t x,
			uint32_t y,
			uint32_t width,
			uint32_t height,
			PixelBufferDescriptor&& data)
		{

		}

		void D3D11Driver::blit(
			TargetBufferFlags buffers,
			Handle<HwRenderTarget> dst,
			backend::Viewport dstRect,
			Handle<HwRenderTarget> src,
			backend::Viewport srcRect,
			SamplerMagFilter filter)
		{
			
		}

		void D3D11Driver::draw(backend::PipelineState ps, Handle<HwRenderPrimitive> rph)
		{
			auto program = handle_cast<D3D11Program>(m_handle_map, ps.program);
			auto primitive = handle_cast<D3D11RenderPrimitive>(m_handle_map, rph);
			auto vertex_buffer = handle_cast<D3D11VertexBuffer>(m_handle_map, primitive->vertex_buffer);
			auto index_buffer = handle_cast<D3D11IndexBuffer>(m_handle_map, primitive->index_buffer);
			
			if (program->vertex_shader)
			{
				m_context->context->VSSetShader(program->vertex_shader, nullptr, 0);

				for (size_t i = 0; i < m_context->uniform_buffer_bindings.size(); ++i)
				{
					if (m_context->uniform_buffer_bindings[i].buffer)
					{
						m_context->context->VSSetConstantBuffers1((UINT) i, 1, &m_context->uniform_buffer_bindings[i].buffer, nullptr, nullptr);
					}
					else
					{
						m_context->context->VSSetConstantBuffers1((UINT) i, 0, nullptr, nullptr, nullptr);
					}
				}
			}
			
			if (program->pixel_shader)
			{
				m_context->context->PSSetShader(program->pixel_shader, nullptr, 0);

				for (size_t i = 0; i < m_context->uniform_buffer_bindings.size(); ++i)
				{
					if (m_context->uniform_buffer_bindings[i].buffer)
					{
						m_context->context->PSSetConstantBuffers1((UINT) i, 1, &m_context->uniform_buffer_bindings[i].buffer, nullptr, nullptr);
					}
					else
					{
						m_context->context->PSSetConstantBuffers1((UINT) i, 0, nullptr, nullptr, nullptr);
					}
				}

				const auto& samplers = program->info.getSamplerGroupInfo();
				for (size_t i = 0; i < samplers.size(); ++i)
				{
					if (m_context->sampler_group_binding[i].sampler_group)
					{
						auto sampler_group = handle_cast<D3D11SamplerGroup>(m_handle_map, m_context->sampler_group_binding[i].sampler_group);

						for (int j = 0; j < samplers[i].size(); ++j)
						{
							auto& s = sampler_group->sb->getSamplers()[j];
							
							if (s.t)
							{
								auto texture = handle_const_cast<D3D11Texture>(m_handle_map, s.t);
								if (texture->image_view)
								{
									m_context->context->PSSetShaderResources((UINT) samplers[i][j].binding, 1, (ID3D11ShaderResourceView* const*) &texture->image_view);

									ID3D11SamplerState* sampler = m_context->GetSampler(s.s);
									m_context->context->PSSetSamplers((UINT) samplers[i][j].binding, 1, &sampler);
								}
							}
						}
					}
				}
			}

			m_context->SetState(ps);

			std::vector<ID3D11Buffer*> buffers;
			std::vector<UINT> strides;
			std::vector<UINT> offsets;

			for (size_t i = 0; i < vertex_buffer->attributes.size(); ++i)
			{
				const auto& attribute = vertex_buffer->attributes[i];
				if (attribute.buffer != 0XFF)
				{
					buffers.push_back(vertex_buffer->buffers[attribute.buffer]);
					strides.push_back(attribute.stride);
					offsets.push_back(attribute.offset);
				}
			}
			
			m_context->context->IASetVertexBuffers(
				0,
				(UINT) buffers.size(),
				&buffers[0],
				&strides[0],
				&offsets[0]);

			DXGI_FORMAT index_format = DXGI_FORMAT_UNKNOWN;
			if (index_buffer->elementSize == 2)
			{
				index_format = DXGI_FORMAT_R16_UINT;
			}
			else if (index_buffer->elementSize == 4)
			{
				index_format = DXGI_FORMAT_R32_UINT;
			}
			m_context->context->IASetIndexBuffer(index_buffer->buffer, index_format, 0);

			D3D11_PRIMITIVE_TOPOLOGY primitive_type = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
			switch (primitive->type)
			{
			case PrimitiveType::POINTS:
				primitive_type = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
				break;
			case PrimitiveType::LINES:
				primitive_type = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
				break;
			case PrimitiveType::TRIANGLES:
				primitive_type = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
				break;
			default:
				assert(false);
				break;
			}
			m_context->context->IASetPrimitiveTopology(primitive_type);

			if (program->input_layout == nullptr)
			{
				auto get_format = [](ElementType type) {
					switch (type)
					{
					case ElementType::FLOAT2: return DXGI_FORMAT_R32G32_FLOAT;
					case ElementType::FLOAT3: return DXGI_FORMAT_R32G32B32_FLOAT;
					case ElementType::FLOAT4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
					default: assert(false); return DXGI_FORMAT_UNKNOWN;
					}
				};

				std::vector<D3D11_INPUT_ELEMENT_DESC> input_descs;
				for (size_t i = 0; i < vertex_buffer->attributes.size(); ++i)
				{
					if (primitive->enabled_attributes & (1 << i))
					{
						const auto& attribute = vertex_buffer->attributes[i];
						D3D11_INPUT_ELEMENT_DESC desc = { };
						desc.SemanticName = "TEXCOORD";
						desc.SemanticIndex = (UINT) i;
						desc.Format = get_format(attribute.type);
						desc.InputSlot = (UINT) i;
						desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

						input_descs.push_back(desc);
					}
				}

				HRESULT hr = m_context->device->CreateInputLayout(
					&input_descs[0],
					(UINT) input_descs.size(),
					program->vertex_binary->GetBufferPointer(),
					program->vertex_binary->GetBufferSize(),
					&program->input_layout);
				assert(SUCCEEDED(hr));
			}
			m_context->context->IASetInputLayout(program->input_layout);

			m_context->context->DrawIndexed(primitive->count, primitive->offset, 0);
		}
	}
}
