/*
* Viry3D
* Copyright 2014-2018 by Stack - stackos@qq.com
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

#include "RenderPass.h"
#include "Camera.h"
#include "FrameBuffer.h"
#include "RenderTexture.h"

namespace Viry3D
{
	RenderPass* RenderPass::m_render_pass_binding;

	Ref<RenderPass> RenderPass::Create(Ref<RenderTexture> color_texture, Ref<RenderTexture> depth_texture, CameraClearFlags clear_flag, bool need_depth, Rect rect)
	{
		Ref<RenderPass> pass = Ref<RenderPass>(new RenderPass());
		pass->m_frame_buffer.color_texture = color_texture;
		pass->m_frame_buffer.depth_texture = depth_texture;
		pass->m_clear_flag = clear_flag;
		pass->m_need_depth = need_depth;
		pass->m_rect = rect;

		pass->CreateInternal();

		return pass;
	}

	RenderPass::RenderPass()
	{
	}

	void RenderPass::Begin(const Color& clear_color)
	{
		Bind();

#if VR_VULKAN
		RenderPassVulkan::Begin(clear_color);
#elif VR_GLES
		RenderPassGLES::Begin(clear_color);
#endif
	}

	void RenderPass::End()
	{
#if VR_VULKAN
		RenderPassVulkan::End();
#elif VR_GLES
		RenderPassGLES::End();
#endif

		Unbind();
	}

	void RenderPass::Bind()
	{
		m_render_pass_binding = this;
	}

	void RenderPass::Unbind()
	{
		m_render_pass_binding = NULL;
	}

	bool RenderPass::HasFrameBuffer() const
	{
		return m_frame_buffer.color_texture || m_frame_buffer.depth_texture;
	}

	int RenderPass::GetFrameBufferWidth() const
	{
		if (!this->HasFrameBuffer())
		{
			return Camera::Current()->GetTargetWidth();
		}
		else
		{
			if (m_frame_buffer.color_texture)
			{
				return m_frame_buffer.color_texture->GetWidth();
			}

			if (m_frame_buffer.depth_texture)
			{
				return m_frame_buffer.depth_texture->GetWidth();
			}
		}

		return -1;
	}

	int RenderPass::GetFrameBufferHeight() const
	{
		if (!this->HasFrameBuffer())
		{
			return Camera::Current()->GetTargetHeight();
		}
		else
		{
			if (m_frame_buffer.color_texture)
			{
				return m_frame_buffer.color_texture->GetHeight();
			}

			if (m_frame_buffer.depth_texture)
			{
				return m_frame_buffer.depth_texture->GetHeight();
			}
		}

		return -1;
	}
}
