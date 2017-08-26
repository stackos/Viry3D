#pragma once

#include "gles_include.h"
#include "graphics/Color.h"

namespace Viry3D
{
	class RenderPassGLES
	{
	public:
		virtual ~RenderPassGLES();
		void Begin(const Color& clear_color);
		void End();
		void* GetCommandBuffer() const { return 0; }
		bool IsCommandDirty() const { return true; }
		bool IsAllCommandDirty() const { return true; }
		void SetCommandDirty() { }

	protected:
		RenderPassGLES();
		void CreateInternal();

	private:
		GLuint m_framebuffer;
	};
}
