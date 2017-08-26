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
