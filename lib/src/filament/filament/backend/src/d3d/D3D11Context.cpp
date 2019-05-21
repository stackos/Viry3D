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

#include "D3D11Context.h"
#include <assert.h>

namespace filament
{
	namespace backend
	{
		static bool SDKLayersAvailable()
		{
			HRESULT hr = D3D11CreateDevice(
				nullptr,
				D3D_DRIVER_TYPE_NULL,       // 无需创建实际硬件设备。
				0,
				D3D11_CREATE_DEVICE_DEBUG,  // 请检查 SDK 层。
				nullptr,                    // 任何功能级别都会这样。
				0,
				D3D11_SDK_VERSION,          // 对于 Microsoft Store 应用，始终将此值设置为 D3D11_SDK_VERSION。
				nullptr,                    // 无需保留 D3D 设备引用。
				nullptr,                    // 无需知道功能级别。
				nullptr                     // 无需保留 D3D 设备上下文引用。
			);

			return SUCCEEDED(hr);
		}

		D3D11Context::D3D11Context()
		{
			UINT creation_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
			sdk_layers_available = SDKLayersAvailable();
			if (sdk_layers_available)
			{
				creation_flags |= D3D11_CREATE_DEVICE_DEBUG;
			}
#endif

			D3D_FEATURE_LEVEL feature_levels[] =
			{
				D3D_FEATURE_LEVEL_12_1,
				D3D_FEATURE_LEVEL_12_0,
				D3D_FEATURE_LEVEL_11_1,
				D3D_FEATURE_LEVEL_11_0,
				D3D_FEATURE_LEVEL_10_1,
				D3D_FEATURE_LEVEL_10_0,
				D3D_FEATURE_LEVEL_9_3,
				D3D_FEATURE_LEVEL_9_2,
				D3D_FEATURE_LEVEL_9_1
			};

			ID3D11DeviceContext* context_0 = nullptr;
			ID3D11Device* device_0 = nullptr;
			HRESULT hr = D3D11CreateDevice(
				nullptr,					// 指定 nullptr 以使用默认适配器。
				D3D_DRIVER_TYPE_HARDWARE,	// 创建使用硬件图形驱动程序的设备。
				0,							// 应为 0，除非驱动程序是 D3D_DRIVER_TYPE_SOFTWARE。
				creation_flags,				// 设置调试和 Direct2D 兼容性标志。
				feature_levels,				// 此应用程序可以支持的功能级别的列表。
				ARRAYSIZE(feature_levels),	// 上面的列表的大小。
				D3D11_SDK_VERSION,			// 对于 Microsoft Store 应用，始终将此值设置为 D3D11_SDK_VERSION。
				&device_0,					// 返回创建的 Direct3D 设备。
				&feature_level,				// 返回所创建设备的功能级别。
				&context_0					// 返回设备的即时上下文。
			);
			
			if (FAILED(hr))
			{
				hr = D3D11CreateDevice(
					nullptr,
					D3D_DRIVER_TYPE_WARP, // 创建 WARP 设备而不是硬件设备。
					0,
					creation_flags,
					feature_levels,
					ARRAYSIZE(feature_levels),
					D3D11_SDK_VERSION,
					&device_0,
					&feature_level,
					&context_0
				);

				assert(SUCCEEDED(hr));
			}

			device_0->QueryInterface(__uuidof(ID3D11Device3), (void**) &device);
			context_0->QueryInterface(__uuidof(ID3D11DeviceContext3), (void**) &context);
			
			SAFE_RELEASE(device_0);
			SAFE_RELEASE(context_0);
		}

		D3D11Context::~D3D11Context()
		{
			for (auto i : rasterizer_states)
			{
				SAFE_RELEASE(i.second.raster);
				SAFE_RELEASE(i.second.blend);
				SAFE_RELEASE(i.second.depth);
			}
			rasterizer_states.clear();

			SAFE_RELEASE(device);
			SAFE_RELEASE(context);
		}
	}
}
