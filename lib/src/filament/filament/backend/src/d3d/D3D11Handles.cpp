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

#include "D3D11Handles.h"
#include <d3dcompiler.h>
#include <assert.h>

#if VR_UWP
namespace Viry3D
{
	extern void GetCoreWindowSize(void* window, int* width, int* height);
}
#endif

namespace filament
{
	namespace backend
	{
		D3D11SwapChain::D3D11SwapChain(D3D11Context* context, void* native_window)
		{
			int window_width = 0;
			int window_height = 0;

			DXGI_SWAP_CHAIN_DESC1 desc = { };

#if VR_UWP
			Viry3D::GetCoreWindowSize(native_window, &window_width, &window_height);
#else
			HWND window = reinterpret_cast<HWND>(native_window);
			RECT rect;
			GetClientRect(window, &rect);
			window_width = rect.right - rect.left;
			window_height = rect.bottom - rect.top;
#endif

			desc.Width = window_width;
			desc.Height = window_height;
			desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			desc.Stereo = false;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			desc.BufferCount = 2;
			desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
			desc.Flags = 0;
			desc.Scaling = DXGI_SCALING_NONE;
			desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

			IDXGIDevice3* dxgi_device = nullptr;
			context->device->QueryInterface(__uuidof(IDXGIDevice3), (void**) &dxgi_device);
			IDXGIAdapter* dxgi_adapter = nullptr;
			dxgi_device->GetAdapter(&dxgi_adapter);
			IDXGIFactory4* dxgi_factory = nullptr;
			dxgi_adapter->GetParent(IID_PPV_ARGS(&dxgi_factory));
			IDXGISwapChain1* swap_chain_1 = nullptr;

#if VR_UWP
			HRESULT hr = dxgi_factory->CreateSwapChainForCoreWindow(
				context->device,
				reinterpret_cast<IUnknown*>(native_window),
				&desc,
				nullptr,
				&swap_chain_1);
#else
			HRESULT hr = dxgi_factory->CreateSwapChainForHwnd(
				context->device,
				window,
				&desc,
				nullptr,
				nullptr,
				&swap_chain_1);
			assert(SUCCEEDED(hr));

			hr = dxgi_factory->MakeWindowAssociation(window, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
#endif
			assert(SUCCEEDED(hr));

			swap_chain_1->QueryInterface(__uuidof(IDXGISwapChain3), (void**) &swap_chain);

			dxgi_device->SetMaximumFrameLatency(1);

			SAFE_RELEASE(swap_chain_1);
			SAFE_RELEASE(dxgi_factory);
			SAFE_RELEASE(dxgi_adapter);
			SAFE_RELEASE(dxgi_device);

			ID3D11Texture2D1* back_buffer = nullptr;
			hr = swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
			assert(SUCCEEDED(hr));

			context->device->CreateRenderTargetView1(
				back_buffer,
				nullptr,
				&color_view);

			SAFE_RELEASE(back_buffer);
		}

		D3D11SwapChain::~D3D11SwapChain()
		{
			SAFE_RELEASE(color_view);
			SAFE_RELEASE(swap_chain);
		}

		D3D11RenderTarget::D3D11RenderTarget(
			D3D11Context* context,
			TargetBufferFlags flags,
			uint32_t width,
			uint32_t height,
			uint8_t samples,
			TargetBufferInfo color,
			TargetBufferInfo depth,
			TargetBufferInfo stencil):
			HwRenderTarget(width, height)
		{
			
		}

		D3D11RenderTarget::D3D11RenderTarget(D3D11Context* context):
			HwRenderTarget(0, 0)
		{
			default_render_target = true;
		}

		D3D11RenderTarget::~D3D11RenderTarget()
		{
			SAFE_RELEASE(color_view);
			SAFE_RELEASE(depth_view);
		}

		void D3D11RenderTarget::CreateDepth(D3D11Context* context, DXGI_FORMAT format, uint32_t width, uint32_t height)
		{
			CD3D11_TEXTURE2D_DESC1 texture_desc(
				format, 
				width,
				height,
				1,
				1,
				D3D11_BIND_DEPTH_STENCIL
			);

			ID3D11Texture2D1* texture = nullptr;
			HRESULT hr = context->device->CreateTexture2D1(
				&texture_desc,
				nullptr,
				&texture);
			assert(SUCCEEDED(hr));

			CD3D11_DEPTH_STENCIL_VIEW_DESC depth_view_desc(D3D11_DSV_DIMENSION_TEXTURE2D);
			hr = context->device->CreateDepthStencilView(
				texture,
				&depth_view_desc,
				&depth_view);
			assert(SUCCEEDED(hr));

			SAFE_RELEASE(texture);
		}

		D3D11Program::D3D11Program(D3D11Context* context, Program&& program):
			HwProgram(program.getName())
		{
			const auto& sources = program.getShadersSource();
			for (int i = 0; i < sources.size(); ++i)
			{
				std::string src((const char*) &sources[i][0], sources[i].size());
				auto type = (Program::Shader) i;

				const char* target = nullptr;
				if (type == Program::Shader::VERTEX)
				{
					//target = "vs_4_0_level_9_3";
					target = "vs_4_0";
				}
				else if (type == Program::Shader::FRAGMENT)
				{
					//target = "ps_4_0_level_9_3";
					target = "ps_4_0";
				}

				ID3DBlob* binary = nullptr;
				ID3DBlob* error = nullptr;

				HRESULT hr = D3DCompile(
					&src[0],
					src.size(),
					program.getName().c_str(),
					nullptr,
					nullptr,
					"main",
					target,
					0,
					0,
					&binary,
					&error);

				if (error)
				{
					std::string message = reinterpret_cast<const char*>(error->GetBufferPointer());
					assert(!error);
				}

				if (type == Program::Shader::VERTEX)
				{
					hr = context->device->CreateVertexShader(
						binary->GetBufferPointer(),
						binary->GetBufferSize(),
						nullptr,
						&vertex_shader);
					assert(SUCCEEDED(hr));

					vertex_binary = binary;
				}
				else if (type == Program::Shader::FRAGMENT)
				{
					hr = context->device->CreatePixelShader(
						binary->GetBufferPointer(),
						binary->GetBufferSize(),
						nullptr,
						&pixel_shader);
					assert(SUCCEEDED(hr));

					pixel_binary = binary;
				}

				SAFE_RELEASE(error);
			}

			info = std::move(program);
		}

		D3D11Program::~D3D11Program()
		{
			SAFE_RELEASE(vertex_binary);
			SAFE_RELEASE(pixel_binary);
			SAFE_RELEASE(vertex_shader);
			SAFE_RELEASE(pixel_shader);
			SAFE_RELEASE(input_layout);
		}

		D3D11UniformBuffer::D3D11UniformBuffer(D3D11Context* context, size_t size, BufferUsage usage):
			size(size),
			usage(usage)
		{
			CD3D11_BUFFER_DESC buffer_desc((UINT) size, D3D11_BIND_CONSTANT_BUFFER);
			
			switch (usage)
			{
			case BufferUsage::STATIC:
				buffer_desc.Usage = D3D11_USAGE_DEFAULT;
				break;
			case BufferUsage::DYNAMIC:
				buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
				buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				break;
			default:
				assert(false);
				break;
			}

			HRESULT hr = context->device->CreateBuffer(&buffer_desc, nullptr, &buffer);
			assert(SUCCEEDED(hr));
		}

		D3D11UniformBuffer::~D3D11UniformBuffer()
		{
			SAFE_RELEASE(buffer);
		}

		void D3D11UniformBuffer::Load(D3D11Context* context, const BufferDescriptor& data)
		{
			if (usage == BufferUsage::DYNAMIC)
			{
				D3D11_MAPPED_SUBRESOURCE res = { };
				HRESULT hr = context->context->Map(
					buffer,
					0,
					D3D11_MAP_WRITE_DISCARD,
					0,
					&res);
				assert(SUCCEEDED(hr));

				memcpy(res.pData, data.buffer, data.size);

				context->context->Unmap(buffer, 0);
			}
			else
			{
				UINT offset = 0;
				D3D11_BOX box = { offset, 0, 0, offset + (UINT) data.size, 1, 1 };
				context->context->UpdateSubresource1(
					buffer,
					0,
					&box,
					data.buffer,
					0,
					0,
					0);
			}
		}

		D3D11SamplerGroup::D3D11SamplerGroup(D3D11Context* context, size_t size):
			HwSamplerGroup(size)
		{
		
		}

		D3D11SamplerGroup::~D3D11SamplerGroup()
		{
			
		}

		void D3D11SamplerGroup::Update(D3D11Context* context, SamplerGroup&& sg)
		{
			*sb = std::move(sg);
		}

		D3D11Texture::D3D11Texture(
			D3D11Context* context,
			backend::SamplerType target,
			uint8_t levels,
			TextureFormat format,
			uint8_t samples,
			uint32_t width,
			uint32_t height,
			uint32_t depth,
			TextureUsage usage):
			HwTexture(target, levels, samples, width, height, depth, format, usage)
		{
			UINT mip_levels = levels;
			UINT array_size = depth;
			UINT bind_flags = 0;
			UINT misc_flags = 0;

			if (usage & TextureUsage::COLOR_ATTACHMENT)
			{
				bind_flags |= D3D11_BIND_RENDER_TARGET;
			}
			if ((usage & TextureUsage::DEPTH_ATTACHMENT) ||
				(usage & TextureUsage::STENCIL_ATTACHMENT))
			{
				bind_flags |= D3D11_BIND_DEPTH_STENCIL;
			}
			if (usage & TextureUsage::SAMPLEABLE)
			{
				bind_flags |= D3D11_BIND_SHADER_RESOURCE;
			}

			if (levels > 1)
			{
				mip_levels = 0;
				bind_flags |= D3D11_BIND_RENDER_TARGET;
				misc_flags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
			}

			if (target == backend::SamplerType::SAMPLER_CUBEMAP)
			{
				array_size = depth * 6;
				misc_flags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
			}

			D3D11_TEXTURE2D_DESC1 texture_desc = { };
			texture_desc.Width = width;
			texture_desc.Height = height;
			texture_desc.MipLevels = mip_levels;
			texture_desc.ArraySize = array_size;
			texture_desc.Format = context->GetTextureFormat(format);
			texture_desc.SampleDesc.Count = samples;
			texture_desc.Usage = D3D11_USAGE_DEFAULT;
			texture_desc.BindFlags = bind_flags;
			texture_desc.CPUAccessFlags = 0;
			texture_desc.MiscFlags = misc_flags;
			texture_desc.TextureLayout = D3D11_TEXTURE_LAYOUT_UNDEFINED;

			HRESULT hr = context->device->CreateTexture2D1(
				&texture_desc,
				nullptr,
				&texture);
			assert(SUCCEEDED(hr));

			if (usage & TextureUsage::SAMPLEABLE)
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC1 view_desc = { };
				view_desc.Format = context->GetTextureViewFormat(format);

				if (target == backend::SamplerType::SAMPLER_CUBEMAP)
				{
					view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
					view_desc.TextureCube.MipLevels = -1;
				}
				else
				{
					view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
					view_desc.Texture2D.MipLevels = -1;
				}

				hr = context->device->CreateShaderResourceView1(texture, &view_desc, &image_view);
				assert(SUCCEEDED(hr));
			}
		}

		D3D11Texture::~D3D11Texture()
		{
			SAFE_RELEASE(texture);
			SAFE_RELEASE(image_view);
		}

		void D3D11Texture::UpdateTexture(
			D3D11Context* context,
			int layer, int level,
			int x, int y,
			int w, int h,
			const PixelBufferDescriptor& data)
		{
			UINT subresource = D3D11CalcSubresource(level, layer, levels);
			D3D11_BOX box = { (UINT) x, (UINT) y, 0, (UINT) (x + w), (UINT) (y + h), 1 };
			UINT row_pitch = (UINT) getTextureFormatSize(format) * w;

			context->context->UpdateSubresource1(
				texture,
				subresource,
				&box,
				data.buffer,
				row_pitch,
				0,
				D3D11_COPY_DISCARD);
		}

		void D3D11Texture::CopyTexture(
			D3D11Context* context,
			int dst_layer, int dst_level,
			const backend::Offset3D& dst_offset,
			const backend::Offset3D& dst_extent,
			D3D11Texture* src,
			int src_layer, int src_level,
			const backend::Offset3D& src_offset,
			const backend::Offset3D& src_extent)
		{
			assert(dst_extent.x == src_extent.x);
			assert(dst_extent.y == src_extent.y);
			assert(dst_extent.z == src_extent.z);

			UINT dst_subresource = D3D11CalcSubresource(dst_level, dst_layer, levels);
			UINT src_subresource = D3D11CalcSubresource(src_level, src_layer, levels);
			D3D11_BOX box = {
				(UINT) src_offset.x,
				(UINT) src_offset.y,
				(UINT) src_offset.z,
				(UINT) (src_offset.x + src_extent.x),
				(UINT) (src_offset.y + src_extent.y),
				(UINT) (src_offset.z + src_extent.z)
			};

			context->context->CopySubresourceRegion(
				texture,
				dst_subresource,
				dst_offset.x,
				dst_offset.y,
				dst_offset.z,
				src->texture,
				src_subresource,
				&box);
		}

		void D3D11Texture::CopyTextureToMemory(
			D3D11Context* context,
			int layer, int level,
			const Offset3D& offset,
			const Offset3D& extent,
			PixelBufferDescriptor& data)
		{
			D3D11_TEXTURE2D_DESC1 texture_desc = { };
			texture_desc.Width = extent.x;
			texture_desc.Height = extent.y;
			texture_desc.MipLevels = 1;
			texture_desc.ArraySize = 1;
			texture_desc.Format = context->GetTextureFormat(format);
			texture_desc.SampleDesc.Count = 1;
			texture_desc.Usage = D3D11_USAGE_STAGING;
			texture_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

			ID3D11Texture2D1* staging_texture = nullptr;
			HRESULT hr = context->device->CreateTexture2D1(
				&texture_desc,
				nullptr,
				&staging_texture);
			assert(SUCCEEDED(hr));

			// copy
			UINT dst_subresource = D3D11CalcSubresource(0, 0, 1);
			UINT src_subresource = D3D11CalcSubresource(level, layer, levels);
			D3D11_BOX box = {
				(UINT) offset.x,
				(UINT) offset.y,
				(UINT) offset.z,
				(UINT) (offset.x + extent.x),
				(UINT) (offset.y + extent.y),
				(UINT) (offset.z + extent.z)
			};

			context->context->CopySubresourceRegion(
				staging_texture,
				dst_subresource,
				0, 0, 0,
				texture,
				src_subresource,
				&box);

			// map
			D3D11_MAPPED_SUBRESOURCE res = { };
			D3D11_MAP map_type = D3D11_MAP_READ;
			hr = context->context->Map(
				staging_texture,
				dst_subresource,
				map_type,
				0,
				&res);
			assert(SUCCEEDED(hr));

			uint8_t* p = (uint8_t*) res.pData;
			memcpy(data.buffer, p, data.size);

			context->context->Unmap(staging_texture, dst_subresource);

			SAFE_RELEASE(staging_texture);
		}

		void D3D11Texture::GenerateMipmaps(D3D11Context* context)
		{
			context->context->GenerateMips(image_view);
		}

		D3D11VertexBuffer::D3D11VertexBuffer(
			D3D11Context* context,
			uint8_t buffer_count,
			uint8_t attribute_count,
			uint32_t vertex_count,
			AttributeArray attributes,
			BufferUsage usage):
			HwVertexBuffer(buffer_count, attribute_count, vertex_count, attributes),
			buffers(buffer_count, nullptr),
			usage(usage)
		{

		}

		D3D11VertexBuffer::~D3D11VertexBuffer()
		{
			for (int i = 0; i < buffers.size(); ++i)
			{
				SAFE_RELEASE(buffers[i]);
			}
		}

		void D3D11VertexBuffer::Update(
			D3D11Context* context,
			size_t index,
			const BufferDescriptor& data,
			uint32_t offset)
		{
			assert(index < buffers.size());

			if (buffers[index])
			{
				D3D11_BUFFER_DESC buffer_desc = { };
				buffers[index]->GetDesc(&buffer_desc);

				assert(offset + data.size <= buffer_desc.ByteWidth);
			}
			else
			{
				CD3D11_BUFFER_DESC buffer_desc((UINT) (offset + data.size), D3D11_BIND_VERTEX_BUFFER);

				switch (usage)
				{
				case BufferUsage::STATIC:
					buffer_desc.Usage = D3D11_USAGE_DEFAULT;
					break;
				case BufferUsage::DYNAMIC:
					buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
					buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
					break;
				default:
					assert(false);
					break;
				}

				HRESULT hr = context->device->CreateBuffer(&buffer_desc, nullptr, &buffers[index]);
				assert(SUCCEEDED(hr));
			}

			if (usage == BufferUsage::DYNAMIC)
			{
				D3D11_MAPPED_SUBRESOURCE res = { };
				HRESULT hr = context->context->Map(
					buffers[index],
					0,
					D3D11_MAP_WRITE_DISCARD,
					0,
					&res);
				assert(SUCCEEDED(hr));

				uint8_t* p = (uint8_t*) res.pData;
				memcpy(&p[offset], data.buffer, data.size);

				context->context->Unmap(buffers[index], 0);
			}
			else
			{
				D3D11_BOX box = { offset, 0, 0, offset + (UINT) data.size, 1, 1 };
				context->context->UpdateSubresource1(
					buffers[index],
					0,
					&box,
					data.buffer,
					0,
					0,
					0);
			}
		}

		D3D11IndexBuffer::D3D11IndexBuffer(
			D3D11Context* context,
			ElementType element_type,
			uint32_t index_count,
			BufferUsage usage):
			HwIndexBuffer((uint8_t) D3D11Driver::getElementTypeSize(element_type), index_count),
			usage(usage)
		{
			CD3D11_BUFFER_DESC buffer_desc((UINT) (elementSize * index_count), D3D11_BIND_INDEX_BUFFER);

			switch (usage)
			{
			case BufferUsage::STATIC:
				buffer_desc.Usage = D3D11_USAGE_DEFAULT;
				break;
			case BufferUsage::DYNAMIC:
				buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
				buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				break;
			default:
				assert(false);
				break;
			}

			HRESULT hr = context->device->CreateBuffer(&buffer_desc, nullptr, &buffer);
			assert(SUCCEEDED(hr));
		}

		D3D11IndexBuffer::~D3D11IndexBuffer()
		{
			SAFE_RELEASE(buffer);
		}

		void D3D11IndexBuffer::Update(
			D3D11Context* context,
			const BufferDescriptor& data,
			uint32_t offset)
		{
			if (usage == BufferUsage::DYNAMIC)
			{
				D3D11_MAPPED_SUBRESOURCE res = { };
				HRESULT hr = context->context->Map(
					buffer,
					0,
					D3D11_MAP_WRITE_DISCARD,
					0,
					&res);
				assert(SUCCEEDED(hr));

				uint8_t* p = (uint8_t*) res.pData;
				memcpy(&p[offset], data.buffer, data.size);

				context->context->Unmap(buffer, 0);
			}
			else
			{
				D3D11_BOX box = { offset, 0, 0, offset + (UINT) data.size, 1, 1 };
				context->context->UpdateSubresource1(
					buffer,
					0,
					&box,
					data.buffer,
					0,
					0,
					0);
			}
		}

		D3D11RenderPrimitive::D3D11RenderPrimitive(D3D11Context* context)
		{
		
		}

		D3D11RenderPrimitive::~D3D11RenderPrimitive()
		{
			
		}

		void D3D11RenderPrimitive::SetBuffer(
			D3D11Context* context,
			Handle<HwVertexBuffer> vbh,
			Handle<HwIndexBuffer> ibh,
			uint32_t enabled_attributes,
			uint32_t vertex_count)
		{
			this->vertex_buffer = vbh;
			this->index_buffer = ibh;
			this->enabled_attributes = enabled_attributes;
			this->maxVertexCount = vertex_count;
		}

		void D3D11RenderPrimitive::SetRange(
			D3D11Context* context,
			PrimitiveType pt,
			uint32_t offset,
			uint32_t min_index,
			uint32_t max_index,
			uint32_t count)
		{
			this->offset = offset;
			this->minIndex = min_index;
			this->maxIndex = max_index;
			this->count = count;
			this->type = pt;
		}
	}
}