/*
* Viry3D
* Copyright 2014-2017 by Stack - stackos@qq.com
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

#include "vulkan_include.h"
#include "memory/Ref.h"
#include "container/Vector.h"
#include "graphics/Color.h"

namespace Viry3D
{
	class RenderPassVulkan
	{
	public:
		virtual ~RenderPassVulkan();
		void Begin(const Color& clear_color);
		void End();
		VkRenderPass GetVkRenderPass() const { return m_render_pass; }
		VkCommandBuffer GetCommandBuffer() const;

	protected:
		RenderPassVulkan();
		void CreateInternal();

	private:
		struct CommandBuffer
		{
			VkFramebuffer frame_buffer;
			VkCommandBuffer cmd_buffer;
			int draw_call;
		};

		VkRenderPass m_render_pass;
		Vector<CommandBuffer> m_framebuffers;
	};
}
