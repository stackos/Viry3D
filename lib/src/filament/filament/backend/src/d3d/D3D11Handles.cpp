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
#include <wrl.h>
using namespace Windows::UI::Core;
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
			CoreWindow^ window = reinterpret_cast<CoreWindow^>(native_window);
			window_width = (int) window->Bounds.Width;
			window_height = (int) window->Bounds.Height;
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
				reinterpret_cast<HWND>(native_window),
				&desc,
				nullptr,
				nullptr,
				&swap_chain_1);
#endif
			assert(SUCCEEDED(hr));

			swap_chain_1->QueryInterface(__uuidof(IDXGISwapChain3), (void**) &swap_chain);

			dxgi_device->SetMaximumFrameLatency(1);

			SAFE_RELEASE(swap_chain_1);
			SAFE_RELEASE(dxgi_factory);
			SAFE_RELEASE(dxgi_adapter);
			SAFE_RELEASE(dxgi_device);

			ID3D11Texture2D1* back_buffer = nullptr;
			swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));

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
			uint32_t width,
			uint32_t height,
			ID3D11Texture2D1* color,
			ID3D11Texture2D1* depth,
			uint8_t level):
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
					target = "vs_4_0_level_9_3";
				}
				else if (type == Program::Shader::FRAGMENT)
				{
					target = "ps_4_0_level_9_3";
				}

				ID3DBlob *binary = nullptr;
				ID3DBlob *error = nullptr;

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
				}
				else if (type == Program::Shader::FRAGMENT)
				{
					hr = context->device->CreatePixelShader(
						binary->GetBufferPointer(),
						binary->GetBufferSize(),
						nullptr,
						&pixel_shader);
					assert(SUCCEEDED(hr));
				}

				SAFE_RELEASE(binary);
				SAFE_RELEASE(error);
			}
		}

		D3D11Program::~D3D11Program()
		{
			SAFE_RELEASE(vertex_shader);
			SAFE_RELEASE(pixel_shader);
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

		void D3D11UniformBuffer::Load(D3D11Context* context, BufferDescriptor&& data)
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
			HwSamplerGroup(size),
			samplers(size, nullptr)
		{
		
		}

		D3D11SamplerGroup::~D3D11SamplerGroup()
		{
			for (int i = 0; i < samplers.size(); ++i)
			{
				SAFE_RELEASE(samplers[i]);
			}
		}

		void D3D11SamplerGroup::Update(D3D11Context* context, SamplerGroup&& sg)
		{
			assert(samplers.size() == sg.getSize());

			for (int i = 0; i < sb->getSize(); ++i)
			{
				if (sb->getSamplers()[i].s.u != sg.getSamplers()[i].s.u)
				{
					SAFE_RELEASE(samplers[i]);
				}
			}

			*sb = std::move(sg);

			for (int i = 0; i < samplers.size(); ++i)
			{
				if (samplers[i] == nullptr)
				{
					const auto& s = sb->getSamplers()[i].s;

					D3D11_SAMPLER_DESC sampler_desc = { };
					sampler_desc.MaxAnisotropy = 1;
					sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
					sampler_desc.MinLOD = -3.402823466e+38F;
					sampler_desc.MaxLOD = 3.402823466e+38F;

					switch (s.filterMag)
					{
					case SamplerMagFilter::NEAREST:
						sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
						break;
					case SamplerMagFilter::LINEAR:
						sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
						break;
					default:
						assert(false);
						break;
					}

					switch (s.wrapS)
					{
					case SamplerWrapMode::CLAMP_TO_EDGE:
						sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
						sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
						sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
						break;
					case SamplerWrapMode::REPEAT:
						sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
						sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
						sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
						break;
					case SamplerWrapMode::MIRRORED_REPEAT:
						sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
						sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
						sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
						break;
					default:
						assert(false);
						break;
					}

					HRESULT hr = context->device->CreateSamplerState(&sampler_desc, &samplers[i]);
					assert(SUCCEEDED(hr));
				}
			}
		}
	}
}