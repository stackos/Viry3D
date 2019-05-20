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

			IDXGIDevice3* dxgi_device;
			context->device->QueryInterface(__uuidof(IDXGIDevice3), (void**) &dxgi_device);
			IDXGIAdapter* dxgi_adapter;
			dxgi_device->GetAdapter(&dxgi_adapter);
			IDXGIFactory4* dxgi_factory;
			dxgi_adapter->GetParent(IID_PPV_ARGS(&dxgi_factory));
			IDXGISwapChain1* swap_chain_1;

#if VR_UWP
			dxgi_factory->CreateSwapChainForCoreWindow(
				context->device,
				reinterpret_cast<IUnknown*>(native_window),
				&desc,
				nullptr,
				&swap_chain_1);
#else
			dxgi_factory->CreateSwapChainForHwnd(
				context->device,
				reinterpret_cast<HWND>(native_window),
				&desc,
				nullptr,
				nullptr,
				&swap_chain_1);
#endif

			swap_chain_1->QueryInterface(__uuidof(IDXGISwapChain3), (void**) &swap_chain);

			dxgi_device->SetMaximumFrameLatency(1);

			SAFE_RELEASE(swap_chain_1);
			SAFE_RELEASE(dxgi_factory);
			SAFE_RELEASE(dxgi_adapter);
			SAFE_RELEASE(dxgi_device);

			ID3D11Texture2D1* back_buffer;
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

			ID3D11Texture2D1* texture;
			context->device->CreateTexture2D1(
				&texture_desc,
				nullptr,
				&texture);

			CD3D11_DEPTH_STENCIL_VIEW_DESC depth_view_desc(D3D11_DSV_DIMENSION_TEXTURE2D);
			context->device->CreateDepthStencilView(
				texture,
				&depth_view_desc,
				&depth_view);

			SAFE_RELEASE(texture);
		}
	}
}