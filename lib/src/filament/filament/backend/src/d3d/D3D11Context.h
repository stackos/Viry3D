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

#include "private/backend/Driver.h"
#include <unordered_map>
#include <dxgi1_4.h>
#include <d3d11_3.h>

#define SAFE_RELEASE(p) \
	if (p) \
	{ \
		p->Release(); \
		p = nullptr; \
	}

namespace filament
{
	namespace backend
	{
		struct D3D11SwapChain;
		struct D3D11RenderTarget;

		class D3D11Context
		{
		public:
			D3D11Context();
			~D3D11Context();
			void SetState(const backend::PipelineState& ps);
			ID3D11SamplerState* GetSampler(const backend::SamplerParams& s);
			DXGI_FORMAT GetTextureFormat(TextureFormat format);
			DXGI_FORMAT GetTextureViewFormat(TextureFormat format);
			DXGI_FORMAT GetDepthViewFormat(TextureFormat format);

			struct UniformBufferBinding
			{
				ID3D11Buffer* buffer = nullptr;
				size_t offset = 0;
				size_t size = 0;
			};
			
			struct SamplerGroupBinding
			{
				SamplerGroupHandle sampler_group;
			};

			struct RenderState
			{
				ID3D11RasterizerState* raster = nullptr;
				ID3D11BlendState* blend = nullptr;
				ID3D11DepthStencilState* depth = nullptr;
			};

		public:
			ID3D11DeviceContext3* context = nullptr;
			ID3D11Device3* device = nullptr;
			D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_12_1;
#if defined(_DEBUG)
			bool sdk_layers_available = false;
#endif
			D3D11SwapChain* current_swap_chain = nullptr;
			D3D11RenderTarget* current_render_target = nullptr;
			RenderPassFlags current_render_pass_flags;
			std::array<UniformBufferBinding, CONFIG_UNIFORM_BINDING_COUNT> uniform_buffer_bindings;
			std::array<SamplerGroupBinding, CONFIG_SAMPLER_BINDING_COUNT> sampler_group_binding;
			std::unordered_map<uint32_t, RenderState> rasterizer_states;
			std::unordered_map<uint32_t, ID3D11SamplerState*> samplers;
		};
	}
}
