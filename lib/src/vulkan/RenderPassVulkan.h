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
		bool IsCommandDirty() const;
		bool IsAllCommandDirty() const;
		void SetCommandDirty();

	protected:
		RenderPassVulkan();
		void CreateInternal();

	private:
		struct CommandBuffer
		{
			VkFramebuffer frame_buffer;
			VkCommandBuffer cmd_buffer;
			bool cmd_dirty;
			int draw_call;
		};

		VkRenderPass m_render_pass;
		Vector<CommandBuffer> m_framebuffers;
	};
}