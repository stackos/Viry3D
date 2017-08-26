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

#if VR_VULKAN
#include "vulkan/RenderPassVulkan.h"
#elif VR_GLES
#include "gles/RenderPassGLES.h"
#endif

#include "FrameBuffer.h"
#include "CameraClearFlags.h"
#include "math/Rect.h"

namespace Viry3D
{
#if VR_VULKAN
	class RenderPass: public RenderPassVulkan
	{
		friend class RenderPassVulkan;
#elif VR_GLES
	class RenderPass: public RenderPassGLES
	{
		friend class RenderPassGLES;
#endif
	public:
		static RenderPass* GetRenderPassBinding() { return m_render_pass_binding; }
		static Ref<RenderPass> Create(Ref<RenderTexture> color_texture, Ref<RenderTexture> depth_texture, CameraClearFlags clear_flag, bool need_depth, Rect rect);

		void Begin(const Color& clear_color);
		void End();
		void Bind();
		void Unbind();
		FrameBuffer GetFrameBuffer() const { return m_frame_buffer; }
		int GetFrameBufferWidth() const;
		int GetFrameBufferHeight() const;
		Rect GetRect() const { return m_rect; }
		bool HasFrameBuffer() const;

	private:
		RenderPass();

		static RenderPass* m_render_pass_binding;
		FrameBuffer m_frame_buffer;
		CameraClearFlags m_clear_flag;
		bool m_need_depth;
		Rect m_rect;
	};
}
