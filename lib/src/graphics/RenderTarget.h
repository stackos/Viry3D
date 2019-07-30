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

#include "Texture.h"
#include "container/Map.h"
#include "private/backend/DriverApi.h"

namespace Viry3D
{
	class RenderTargetKey
	{
	public:
		union
		{
			struct
			{
				int width									: 16;
				int height									: 16;
				TextureFormat color_format					: 8;
				TextureFormat depth_format					: 8;
				FilterMode filter_mode						: 4;
				SamplerAddressMode wrap_mode				: 4;
				filament::backend::TargetBufferFlags flags	: 4;
				int padding									: 4;
			};
			uint64_t u = 0;
		};
	};

	class RenderTarget;

	class TemporaryRenderTargets
	{
	public:
		RenderTargetKey key;
		Vector<Ref<RenderTarget>> targets;
	};

	class RenderTarget
	{
	public:
		static void Init();
		static void Done();
		static Ref<RenderTarget> GetTemporaryRenderTarget(
			int width,
			int height,
			TextureFormat color_format,
			TextureFormat depth_format,
			FilterMode filter_mode,
			SamplerAddressMode wrap_mode,
			filament::backend::TargetBufferFlags flags);
		static void ReleaseTemporaryRenderTarget(const Ref<RenderTarget>& target);

	public:
		filament::backend::RenderTargetHandle target;
		Ref<Texture> color;
		Ref<Texture> depth;
		RenderTargetKey key;

	private:
		static Map<uint64_t, TemporaryRenderTargets> m_temporary_render_targets_using;
		static Map<uint64_t, TemporaryRenderTargets> m_temporary_render_targets_idle;
	};
}
