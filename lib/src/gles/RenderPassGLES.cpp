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

#include "RenderPassGLES.h"
#include "graphics/RenderTexture.h"
#include "graphics/Graphics.h"
#include "graphics/RenderPass.h"
#include "graphics/Camera.h"
#include "Debug.h"

#if VR_IOS
#include "ios/DisplayIOS.h"
#endif

#if VR_GLES

namespace Viry3D
{
	RenderPassGLES::RenderPassGLES():
		m_framebuffer(0)
	{
	}

	RenderPassGLES::~RenderPassGLES()
	{
		if (m_framebuffer != 0)
		{
			glDeleteFramebuffers(1, &m_framebuffer);
		}
	}

	void RenderPassGLES::CreateInternal()
	{
		LogGLError();

		auto pass = (RenderPass*) this;

		bool is_default = !pass->HasFrameBuffer();

		if (is_default)
		{
			m_framebuffer = 0;
		}
		else
		{
			auto frame_buffer = pass->m_frame_buffer;

			GLuint depth_texture = 0;
			GLenum attachment = 0;
			if (frame_buffer.depth_texture)
			{
				depth_texture = frame_buffer.depth_texture->GetTexture();

				auto depth = frame_buffer.depth_texture->GetDepth();
				switch (depth)
				{
					case DepthBuffer::Depth_16:
					case DepthBuffer::Depth_24:
					case DepthBuffer::Depth_32:
						attachment = GL_DEPTH_ATTACHMENT;
						break;
					case DepthBuffer::Depth_24_Stencil_8:
						attachment = GL_DEPTH_STENCIL_ATTACHMENT;
						break;
					default:
						assert(!"invalid depth texture");
						break;
				}
			}
			else
			{
                if (frame_buffer.color_texture->GetWidth() == Graphics::GetDisplay()->GetWidth() &&
                    frame_buffer.color_texture->GetHeight() == Graphics::GetDisplay()->GetHeight())
                {
                    depth_texture = (GLuint) ((DisplayGLES*) Graphics::GetDisplay())->GetDefualtDepthRenderBuffer();
                    attachment = GL_DEPTH_STENCIL_ATTACHMENT;
                }
			}

			glGenFramebuffers(1, &m_framebuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

			if (frame_buffer.color_texture)
			{
				auto color_texture = frame_buffer.color_texture->GetTexture();
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_texture, 0);
			}

			if (frame_buffer.depth_texture)
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, depth_texture, 0);
			}
			else if (depth_texture)
			{
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, depth_texture);
			}

			auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (status != GL_FRAMEBUFFER_COMPLETE)
			{
				Log("glCheckFramebufferStatus error:%d", status);
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		LogGLError();
	}

	void RenderPassGLES::Begin(const Color& clear_color)
	{
		LogGLError();

		auto pass = (RenderPass*) this;
		auto clear_flag = pass->m_clear_flag;
		Vector<GLenum> attachments;
		bool has_stencil = false;

		int width = pass->GetFrameBufferWidth();
		int height = pass->GetFrameBufferHeight();

		glViewport(0, 0, width, height);

		if (m_framebuffer == 0)
		{
#if VR_IOS
			((DisplayIOS*) Graphics::GetDisplay())->BindDefaultFramebuffer();
#else
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif

#if VR_IOS
			attachments.Add(GL_COLOR_ATTACHMENT0);
			attachments.Add(GL_DEPTH_ATTACHMENT);
#else
			attachments.Add(GL_COLOR);
			attachments.Add(GL_DEPTH);
#endif
		}
		else
		{
			auto frame_buffer = pass->m_frame_buffer;

			if (frame_buffer.color_texture)
			{
				attachments.Add(GL_COLOR_ATTACHMENT0);
			}

			if (frame_buffer.depth_texture)
			{
				auto depth = frame_buffer.depth_texture->GetDepth();
				switch (depth)
				{
					case DepthBuffer::Depth_16:
					case DepthBuffer::Depth_24:
					case DepthBuffer::Depth_32:
						attachments.Add(GL_DEPTH_ATTACHMENT);
						break;
					case DepthBuffer::Depth_24_Stencil_8:
						attachments.Add(GL_DEPTH_ATTACHMENT);
						attachments.Add(GL_STENCIL_ATTACHMENT);
						has_stencil = true;
						break;
					default:
						assert(!"invalid depth texture");
						break;
				}
			}
			else
			{
				attachments.Add(GL_DEPTH_ATTACHMENT);
			}

			glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
		}

#if VR_WINDOWS || VR_MAC
		if (clear_flag == CameraClearFlags::Invalidate)
		{
			clear_flag = CameraClearFlags::Color;
		}
#endif

		switch (clear_flag)
		{
			case CameraClearFlags::Color:
			{
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				glDepthMask(GL_TRUE);

				GLbitfield clear_bit = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
				if (has_stencil)
				{
					clear_bit |= GL_STENCIL_BUFFER_BIT;
				}

				glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
				glClear(clear_bit);
				break;
			}
			case CameraClearFlags::Depth:
			{
				glDepthMask(GL_TRUE);

				GLbitfield clear_bit = GL_DEPTH_BUFFER_BIT;
				if (has_stencil)
				{
					clear_bit |= GL_STENCIL_BUFFER_BIT;
				}

				glClear(clear_bit);
				break;
			}
			case CameraClearFlags::Nothing:
				break;
			case CameraClearFlags::Invalidate:
#if !VR_MAC
				glInvalidateFramebuffer(GL_FRAMEBUFFER, attachments.Size(), &attachments[0]);
#endif
				break;
		}

		LogGLError();
	}

	void RenderPassGLES::End()
	{
	}
}

#endif
