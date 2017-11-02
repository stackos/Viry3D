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

#include "Object.h"
#include "gles_include.h"

namespace Viry3D
{
	class TextureGLES: public Object
	{
	public:
		virtual ~TextureGLES();
		GLuint GetTexture() const { return m_texture; }
		void UpdateSampler();

	protected:
		TextureGLES();
		void CreateColorRenderTexture();
		void CreateDepthRenderTexture();
		void CreateTexture2D();
		void UpdateTexture2D(int x, int y, int w, int h, const ByteBuffer& colors);
        void SetExternalTexture2D(void* texture);
		void GenerateMipmap();
		void CreateCubemap();
		void UpdateCubemapFace(int face, int level, const ByteBuffer& colors);

	private:
		void Create2D(GLenum format, GLenum type, void* pixels);

	private:
		GLuint m_texture;
		GLenum m_target;
		GLint m_format;
        bool m_external;
	};
}
