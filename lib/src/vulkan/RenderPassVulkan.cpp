#include "RenderPassVulkan.h"
#include "DisplayVulkan.h"
#include "graphics/RenderPass.h"
#include "memory/Memory.h"
#include "graphics/Graphics.h"
#include "Profiler.h"

#if VR_VULKAN

namespace Viry3D
{
	RenderPassVulkan::RenderPassVulkan():
		m_render_pass(NULL)
	{
	}

	RenderPassVulkan::~RenderPassVulkan()
	{
		auto display = (DisplayVulkan*) Graphics::GetDisplay();
		auto device = display->GetDevice();

		vkDeviceWaitIdle(device);

		for(auto i : m_framebuffers)
		{
			vkFreeCommandBuffers(device, display->GetCommandPool(), 1, &i.cmd_buffer);
			vkDestroyFramebuffer(device, i.frame_buffer, NULL);
		}
		vkDestroyRenderPass(device, m_render_pass, NULL);
	}

	void RenderPassVulkan::CreateInternal()
	{
		auto display = (DisplayVulkan*) Graphics::GetDisplay();
		auto device = display->GetDevice();
		auto pass = (RenderPass*) this;
		bool is_default = !pass->HasFrameBuffer();
		auto clear_flag = pass->m_clear_flag;
		bool need_depth = pass->m_need_depth;

		VkFormat color_format;
		VkFormat depth_format;
		VkAttachmentLoadOp color_load;
		VkAttachmentLoadOp depth_load;
		VkImageLayout color_final_layout;

		switch(clear_flag)
		{
			case CameraClearFlags::Color:
			{
				color_load = VK_ATTACHMENT_LOAD_OP_CLEAR;
				depth_load = VK_ATTACHMENT_LOAD_OP_CLEAR;
				break;
			}
			case CameraClearFlags::Depth:
			{
				color_load = VK_ATTACHMENT_LOAD_OP_LOAD;
				depth_load = VK_ATTACHMENT_LOAD_OP_CLEAR;
				break;
			}
			case CameraClearFlags::Invalidate:
				color_load = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				depth_load = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				break;
			default:
			{
				color_load = VK_ATTACHMENT_LOAD_OP_LOAD;
				depth_load = VK_ATTACHMENT_LOAD_OP_LOAD;
				break;
			}
		}

		if(is_default)
		{
			auto depth_texture = display->GetDepthTexture();
			color_format = display->GetSurfaceFormat();
			depth_format = depth_texture->GetVkFormat();
			color_final_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}
		else
		{
			auto frame_buffer = pass->m_frame_buffer;

			if(frame_buffer.color_texture)
			{
				color_format = frame_buffer.color_texture->GetVkFormat();
			}
			else
			{
				color_format = display->GetSurfaceFormat();
			}

			if(frame_buffer.depth_texture)
			{
				depth_format = frame_buffer.depth_texture->GetVkFormat();
			}
			else
			{
				depth_format = display->GetDepthTexture()->GetVkFormat();
			}

			color_final_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		Vector<VkAttachmentDescription> attachments;
		attachments.Add(VkAttachmentDescription());
		Memory::Zero(&attachments[0], sizeof(VkAttachmentDescription));

		attachments[0].format = color_format;
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[0].loadOp = color_load;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout = color_final_layout;

		if(need_depth)
		{
			attachments.Add(VkAttachmentDescription());
			Memory::Zero(&attachments[1], sizeof(VkAttachmentDescription));

			attachments[1].format = depth_format;
			attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[1].loadOp = depth_load;
			attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[1].stencilLoadOp = depth_load;
			attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		VkAttachmentReference color_reference = {
			0,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		};
		VkAttachmentReference depth_reference = {
			1,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		};

		VkSubpassDescription subpass;
		Memory::Zero(&subpass, sizeof(subpass));
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.flags = 0;
		subpass.inputAttachmentCount = 0;
		subpass.pInputAttachments = NULL;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_reference;
		subpass.pResolveAttachments = NULL;
		if(need_depth)
		{
			subpass.pDepthStencilAttachment = &depth_reference;
		}
		subpass.preserveAttachmentCount = 0;
		subpass.pPreserveAttachments = NULL;

		VkRenderPassCreateInfo rp_info;
		Memory::Zero(&rp_info, sizeof(rp_info));
		rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		rp_info.pNext = NULL;
		rp_info.attachmentCount = attachments.Size();
		rp_info.pAttachments = &attachments[0];
		rp_info.subpassCount = 1;
		rp_info.pSubpasses = &subpass;

		VkSubpassDependency dependencies[2];
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		rp_info.dependencyCount = 2;
		rp_info.pDependencies = dependencies;

		VkResult err;
		err = vkCreateRenderPass(device, &rp_info, NULL, &m_render_pass);
		assert(!err);

		//	create frame buffers
		Vector<VkImageView> attachments_view(1);
		Vector<VkImageView> color_views;
		int width;
		int height;

		if(is_default)
		{
			int buffer_count = display->GetSwapchainBufferCount();
			auto depth_texture = display->GetDepthTexture();
			if(need_depth)
			{
				attachments_view.Resize(2);
				attachments_view[1] = depth_texture->GetImageView();
			}
			width = depth_texture->GetWidth();
			height = depth_texture->GetHeight();
			m_framebuffers.Resize(buffer_count);
			color_views.Resize(buffer_count);
			for(int i = 0; i < color_views.Size(); i++)
			{
				color_views[i] = display->GetSwapchainBufferImageView(i);
			}
		}
		else
		{
			auto frame_buffer = pass->m_frame_buffer;
			if(need_depth)
			{
				attachments_view.Resize(2);
				if(frame_buffer.depth_texture)
				{
					attachments_view[1] = frame_buffer.depth_texture->GetImageView();
				}
				else
				{
					attachments_view[1] = display->GetDepthTexture()->GetImageView();
				}
			}
			width = frame_buffer.color_texture->GetWidth();
			height = frame_buffer.color_texture->GetHeight();
			m_framebuffers.Resize(1);
			color_views.Resize(1);
			color_views[0] = frame_buffer.color_texture->GetImageView();
		}

		VkFramebufferCreateInfo fb_info;
		Memory::Zero(&fb_info, sizeof(fb_info));
		fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fb_info.pNext = NULL;
		fb_info.renderPass = m_render_pass;
		fb_info.attachmentCount = attachments_view.Size();
		fb_info.pAttachments = &attachments_view[0];
		fb_info.width = (uint32_t) width;
		fb_info.height = (uint32_t) height;
		fb_info.layers = 1;

		for(int i = 0; i < m_framebuffers.Size(); i++)
		{
			attachments_view[0] = color_views[i];
			err = vkCreateFramebuffer(device, &fb_info, NULL, &m_framebuffers[i].frame_buffer);
			assert(!err);

			VkCommandBufferAllocateInfo cmd_info = {
				VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				NULL,
				display->GetCommandPool(),
				VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				1,
			};
			err = vkAllocateCommandBuffers(device, &cmd_info, &m_framebuffers[i].cmd_buffer);
			assert(!err);

			m_framebuffers[i].cmd_dirty = true;
		}
	}

	VkCommandBuffer RenderPassVulkan::GetCommandBuffer() const
	{
		auto display = Graphics::GetDisplay();
		auto pass = (RenderPass*) this;
		bool is_default = !pass->HasFrameBuffer();
		int swap_index = display->GetSwapBufferIndex();
		if(!is_default)
		{
			swap_index = 0;
		}
		return m_framebuffers[swap_index].cmd_buffer;
	}

	bool RenderPassVulkan::IsCommandDirty() const
	{
		auto display = Graphics::GetDisplay();
		auto pass = (RenderPass*) this;
		bool is_default = !pass->HasFrameBuffer();
		int swap_index = display->GetSwapBufferIndex();
		if(!is_default)
		{
			swap_index = 0;
		}
		return m_framebuffers[swap_index].cmd_dirty;
	}

	bool RenderPassVulkan::IsAllCommandDirty() const
	{
		for(auto& i : m_framebuffers)
		{
			if(!i.cmd_dirty)
			{
				return false;
			}
		}
		return true;
	}

	void RenderPassVulkan::SetCommandDirty()
	{
		for(auto& i : m_framebuffers)
		{
			i.cmd_dirty = true;
		}
	}

	void RenderPassVulkan::Begin(const Color& clear_color)
	{
		VkImage image;
		int width;
		int height;
		VkFramebuffer framebuffer;
		auto display = Graphics::GetDisplay();
		VkCommandBuffer cmd = GetCommandBuffer();
		auto pass = (RenderPass*) this;
		bool is_default = !pass->HasFrameBuffer();
		int swap_index = display->GetSwapBufferIndex();
		if(!is_default)
		{
			swap_index = 0;
		}
		bool need_depth = pass->m_need_depth;

		if(!m_framebuffers[swap_index].cmd_dirty)
		{
			return;
		}

		if(is_default)
		{
			image = display->GetSwapchainBufferImage(swap_index);
			auto depth_texture = display->GetDepthTexture();
			width = depth_texture->GetWidth();
			height = depth_texture->GetHeight();
			framebuffer = m_framebuffers[swap_index].frame_buffer;
		}
		else
		{
			auto frame_buffer = pass->m_frame_buffer;
			image = frame_buffer.color_texture->GetImage();
			width = frame_buffer.color_texture->GetWidth();
			height = frame_buffer.color_texture->GetHeight();
			framebuffer = m_framebuffers[0].frame_buffer;
		}

		VkCommandBufferInheritanceInfo inheritance_info;
		Memory::Zero(&inheritance_info, sizeof(inheritance_info));
		inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritance_info.renderPass = m_render_pass;
		inheritance_info.framebuffer = framebuffer;

		display->BeginPrimaryCommandBuffer(cmd);

		Vector<VkClearValue> clear_values(1);
		clear_values[0].color = *(VkClearColorValue*) &clear_color;

		if(need_depth)
		{
			clear_values.Resize(2);

			VkClearDepthStencilValue clear_depth;
			clear_depth.depth = 1.0f;
			clear_depth.stencil = 0;
			clear_values[1].depthStencil = clear_depth;
		}

		VkRenderPassBeginInfo rp_begin;
		Memory::Zero(&rp_begin, sizeof(rp_begin));
		rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rp_begin.pNext = NULL;
		rp_begin.renderPass = m_render_pass;
		rp_begin.framebuffer = framebuffer;
		rp_begin.renderArea.offset.x = 0;
		rp_begin.renderArea.offset.y = 0;
		rp_begin.renderArea.extent.width = (uint32_t) width;
		rp_begin.renderArea.extent.height = (uint32_t) height;
		rp_begin.clearValueCount = clear_values.Size();
		rp_begin.pClearValues = &clear_values[0];

		vkCmdBeginRenderPass(cmd, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

		m_framebuffers[swap_index].draw_call = Graphics::draw_call;
	}

	void RenderPassVulkan::End()
	{
		auto display = Graphics::GetDisplay();
		auto pass = (RenderPass*) this;
		bool is_default = !pass->HasFrameBuffer();
		int swap_index = display->GetSwapBufferIndex();
		if(!is_default)
		{
			swap_index = 0;
		}

		if(!m_framebuffers[swap_index].cmd_dirty)
		{
			Graphics::draw_call += m_framebuffers[swap_index].draw_call;
			return;
		}

		vkCmdEndRenderPass(GetCommandBuffer());

		display->EndPrimaryCommandBuffer();

		if(m_framebuffers[swap_index].cmd_dirty)
		{
			m_framebuffers[swap_index].cmd_dirty = false;
		}

		m_framebuffers[swap_index].draw_call = Graphics::draw_call - m_framebuffers[swap_index].draw_call;
	}
}

#endif