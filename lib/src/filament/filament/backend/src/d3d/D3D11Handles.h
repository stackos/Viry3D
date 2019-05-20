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

#pragma once

#include "D3D11Driver.h"
#include "D3D11Context.h"

namespace filament
{
	namespace backend
	{
		struct D3D11SwapChain : public HwSwapChain 
		{
			D3D11SwapChain(D3D11Context* context, void* native_window);
			~D3D11SwapChain();

			IDXGISwapChain3* swap_chain = nullptr;
			ID3D11RenderTargetView1* color_view = nullptr;
		};

		struct D3D11RenderTarget : public HwRenderTarget
		{
			D3D11RenderTarget(
				D3D11Context* context,
				uint32_t width,
				uint32_t height,
				ID3D11Texture2D1* color,
				ID3D11Texture2D1* depth,
				uint8_t level);
			explicit D3D11RenderTarget(D3D11Context* context);
			~D3D11RenderTarget();
			void CreateDepth(D3D11Context* context, DXGI_FORMAT format, uint32_t width, uint32_t height);

			bool default_render_target = false;
			ID3D11RenderTargetView1* color_view = nullptr;
			ID3D11DepthStencilView* depth_view = nullptr;
		};
	}
}
