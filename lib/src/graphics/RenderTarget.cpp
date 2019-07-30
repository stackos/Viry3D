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

#include "RenderTarget.h"
#include "Engine.h"

namespace Viry3D
{
	Map<uint64_t, TemporaryRenderTargets> RenderTarget::m_temporary_render_targets_using;
	Map<uint64_t, TemporaryRenderTargets> RenderTarget::m_temporary_render_targets_idle;

	void RenderTarget::Init()
	{

	}

	void RenderTarget::Done()
	{
		auto& driver = Engine::Instance()->GetDriverApi();

		for (const auto& i : m_temporary_render_targets_using)
		{
			const auto& targets = i.second.targets;
			assert(targets.Size() == 0);
		}
		m_temporary_render_targets_using.Clear();

		for (const auto& i : m_temporary_render_targets_idle)
		{
			const auto& targets = i.second.targets;
			for (int j = 0; j < targets.Size(); ++j)
			{
				auto& target = targets[j]->target;
				driver.destroyRenderTarget(target);
				target.clear();
			}
		}
		m_temporary_render_targets_idle.Clear();
	}

	Ref<RenderTarget> RenderTarget::GetTemporaryRenderTarget(
		int width,
		int height,
		TextureFormat color_format,
		TextureFormat depth_format,
		FilterMode filter_mode,
		SamplerAddressMode wrap_mode,
		filament::backend::TargetBufferFlags flags)
	{
		Ref<RenderTarget> target;

		RenderTargetKey key;
		key.width = width;
		key.height = height;
		key.color_format = color_format;
		key.depth_format = depth_format;
		key.filter_mode = filter_mode;
		key.wrap_mode = wrap_mode;
		key.flags = flags;

		TemporaryRenderTargets* p;
		bool create = false;
		if (m_temporary_render_targets_idle.TryGet(key.u, &p))
		{
			if (p->targets.Size() > 0)
			{
				int index = p->targets.Size() - 1;
				target = p->targets[index];
				p->targets.Remove(index);
			}
			else
			{
				create = true;
			}
		}
		else
		{
			create = true;
		}

		if (create)
		{
			target = RefMake<RenderTarget>();
			target->key = key;

			filament::backend::TargetBufferInfo color = { };
			filament::backend::TargetBufferInfo depth = { };
			filament::backend::TargetBufferInfo stencil = { };

			if (flags & filament::backend::TargetBufferFlags::COLOR)
			{
				target->color = Texture::CreateRenderTexture(
					width,
					height,
					color_format,
					filter_mode,
					wrap_mode);
				color.handle = target->color->GetTexture();
			}

			if (flags & filament::backend::TargetBufferFlags::DEPTH)
			{
				target->depth = Texture::CreateRenderTexture(
					width,
					height,
					depth_format,
					filter_mode,
					wrap_mode);

				depth.handle = target->depth->GetTexture();
			}

			auto& driver = Engine::Instance()->GetDriverApi();
			target->target = driver.createRenderTarget(
				flags,
				width,
				height,
				1,
				color,
				depth,
				stencil);
		}

		if (m_temporary_render_targets_using.TryGet(key.u, &p))
		{
			p->targets.Add(target);
		}
		else
		{
			TemporaryRenderTargets targets;
			targets.key = key;
			targets.targets.Add(target);

			m_temporary_render_targets_using.Add(key.u, targets);
		}

		return target;
	}

	void RenderTarget::ReleaseTemporaryRenderTarget(const Ref<RenderTarget>& target)
	{
		RenderTargetKey key = target->key;

		TemporaryRenderTargets* p;
		if (m_temporary_render_targets_using.TryGet(key.u, &p))
		{
			if (p->targets.Remove(target))
			{
				if (m_temporary_render_targets_idle.TryGet(key.u, &p))
				{
					p->targets.Add(target);
				}
				else
				{
					TemporaryRenderTargets targets;
					targets.key = key;
					targets.targets.Add(target);

					m_temporary_render_targets_idle.Add(key.u, targets);
				}
			}
		}
	}
}
