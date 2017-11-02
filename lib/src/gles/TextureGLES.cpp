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

#include "TextureGLES.h"
#include "graphics/RenderTexture.h"
#include "graphics/Texture2D.h"
#include "graphics/Cubemap.h"
#include "Debug.h"
#include "math/Mathf.h"

namespace Viry3D
{
	TextureGLES::TextureGLES():
		m_texture(0),
		m_target(0),
		m_format(0),
        m_external(false)
	{
	}

	TextureGLES::~TextureGLES()
	{
        if (m_external == false)
        {
            glDeleteTextures(1, &m_texture);
        }
	}

	void TextureGLES::CreateColorRenderTexture()
	{
		auto texture = (RenderTexture*) this;
		auto texture_format = texture->GetFormat();

		GLenum format = 0;
		GLenum type = 0;
		if (texture_format == RenderTextureFormat::RGBA32)
		{
			m_format = GL_RGBA;
			format = GL_RGBA;
			type = GL_UNSIGNED_BYTE;
		}
		else if (texture_format == RenderTextureFormat::R8)
		{
			m_format = GL_R8;
			format = GL_RED;
			type = GL_UNSIGNED_BYTE;
		}
		else
		{
			assert(!"color format not invalid");
		}

		this->Create2D(format, type, NULL);
	}

	void TextureGLES::CreateDepthRenderTexture()
	{
		auto texture = (RenderTexture*) this;
		auto depth = texture->GetDepth();

		GLenum format = 0;
		GLenum type = 0;
		if (depth == DepthBuffer::Depth_16)
		{
			m_format = GL_DEPTH_COMPONENT16;
			format = GL_DEPTH_COMPONENT;
			type = GL_UNSIGNED_SHORT;
		}
		else if (depth == DepthBuffer::Depth_24)
		{
			m_format = GL_DEPTH_COMPONENT24;
			format = GL_DEPTH_COMPONENT;
			type = GL_UNSIGNED_INT;
		}
		else if (depth == DepthBuffer::Depth_24_Stencil_8)
		{
			m_format = GL_DEPTH24_STENCIL8;
			format = GL_DEPTH_STENCIL;
			type = GL_UNSIGNED_INT_24_8;
		}
		else if (depth == DepthBuffer::Depth_32)
		{
			m_format = GL_DEPTH_COMPONENT32F;
			format = GL_DEPTH_COMPONENT;
			type = GL_FLOAT;
		}
		else
		{
			assert(!"depth format not invalid");
		}

		this->Create2D(format, type, NULL);
	}

	void TextureGLES::CreateTexture2D()
	{
		auto texture = (Texture2D*) this;
		auto texture_format = texture->GetFormat();
		auto colors = texture->GetColors();

		GLenum format = 0;
		GLenum type = 0;
		if (texture_format == TextureFormat::RGBA32)
		{
			m_format = GL_RGBA;
			format = GL_RGBA;
			type = GL_UNSIGNED_BYTE;
		}
		else if (texture_format == TextureFormat::RGB24)
		{
			m_format = GL_RGB;
			format = GL_RGB;
			type = GL_UNSIGNED_BYTE;
		}
		else if (texture_format == TextureFormat::Alpha8)
		{
			m_format = GL_R8;
			format = GL_RED;
			type = GL_UNSIGNED_BYTE;
		}
		else
		{
			assert(!"texture format not implement");
		}

		this->Create2D(format, type, colors.Bytes());
	}

	void TextureGLES::UpdateTexture2D(int x, int y, int w, int h, const ByteBuffer& colors)
	{
		LogGLError();

		auto texture = (Texture2D*) this;
		auto texture_format = texture->GetFormat();

		GLenum format = 0;
		GLenum type = 0;
		if (texture_format == TextureFormat::RGBA32)
		{
			format = GL_RGBA;
			type = GL_UNSIGNED_BYTE;
		}
		else if (texture_format == TextureFormat::RGB24)
		{
			format = GL_RGB;
			type = GL_UNSIGNED_BYTE;
		}
		else if (texture_format == TextureFormat::Alpha8)
		{
            format = GL_RED;
			type = GL_UNSIGNED_BYTE;
		}
		else
		{
			assert(!"texture format not implement");
		}

		glBindTexture(m_target, m_texture);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexSubImage2D(m_target, 0, x, y, w, h, format, type, colors.Bytes());
		glBindTexture(m_target, 0);

		LogGLError();
	}

	void TextureGLES::Create2D(GLenum format, GLenum type, void* pixels)
	{
		LogGLError();

		auto texture = (Texture*) this;
		int width = texture->GetWidth();
		int height = texture->GetHeight();

		m_target = GL_TEXTURE_2D;

		glGenTextures(1, &m_texture);
		glBindTexture(m_target, m_texture);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(m_target, 0, m_format, width, height, 0, format, type, pixels);

		glBindTexture(m_target, 0);

		LogGLError();

		this->GenerateMipmap();
		this->UpdateSampler();
	}

    void TextureGLES::SetExternalTexture2D(void* texture)
    {
        m_target = GL_TEXTURE_2D;
        m_texture = (GLuint) (size_t) texture;
        m_external = true;
    }

	void TextureGLES::UpdateSampler()
	{
		LogGLError();

		auto texture = (Texture*) this;
		auto wrap_mode = texture->GetWrapMode();
		auto filter_mode = texture->GetFilterMode();
		bool mipmap = texture->IsMipmap();
		GLuint address_mode = 0;
		GLuint filter_min = 0;
		GLuint filter_mag = 0;
		
		switch (wrap_mode)
		{
			case TextureWrapMode::Repeat:
				address_mode = GL_REPEAT;
				break;
			case TextureWrapMode::Clamp:
				address_mode = GL_CLAMP_TO_EDGE;
				break;
		}

		switch (filter_mode)
		{
			case FilterMode::Point:
				if (mipmap)
				{
					filter_min = GL_NEAREST_MIPMAP_NEAREST;
				}
				else
				{
					filter_min = GL_NEAREST;
				}

				filter_mag = GL_NEAREST;
				break;
			case FilterMode::Bilinear:
			case FilterMode::Trilinear:
				if (mipmap)
				{
					filter_min = GL_LINEAR_MIPMAP_LINEAR;
				}
				else
				{
					filter_min = GL_LINEAR;
				}

				filter_mag = GL_LINEAR;
				break;
		}

		glBindTexture(m_target, m_texture);

		glTexParameteri(m_target, GL_TEXTURE_WRAP_S, address_mode);
		glTexParameteri(m_target, GL_TEXTURE_WRAP_T, address_mode);
		glTexParameteri(m_target, GL_TEXTURE_WRAP_R, address_mode);
		glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, filter_mag);
		glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, filter_min);

		glBindTexture(m_target, 0);
        
        LogGLError();
	}

	void TextureGLES::GenerateMipmap()
	{
		LogGLError();

		auto texture = (Texture*) this;
		bool mipmap = texture->IsMipmap();

		if (mipmap)
		{
			glBindTexture(m_target, m_texture);

			glGenerateMipmap(m_target);
		}

		LogGLError();
	}

	void TextureGLES::CreateCubemap()
	{
		LogGLError();

		m_target = GL_TEXTURE_CUBE_MAP;

		glGenTextures(1, &m_texture);
		
		LogGLError();

		this->UpdateSampler();
	}

	void TextureGLES::UpdateCubemapFace(int face, int level, const ByteBuffer& colors)
	{
		LogGLError();

		auto texture = (Cubemap*) this;
		int width = texture->GetWidth();
		int height = texture->GetHeight();
		auto texture_format = texture->GetFormat();

		GLenum format = 0;
		GLenum type = 0;
		if (texture_format == TextureFormat::RGBA32)
		{
			m_format = GL_RGBA;
			format = GL_RGBA;
			type = GL_UNSIGNED_BYTE;
		}
		else if (texture_format == TextureFormat::RGB24)
		{
			m_format = GL_RGB;
			format = GL_RGB;
			type = GL_UNSIGNED_BYTE;
		}
		else if (texture_format == TextureFormat::Alpha8)
		{
			m_format = GL_R8;
			format = GL_RED;
			type = GL_UNSIGNED_BYTE;
		}
		else
		{
			assert(!"texture format not implement");
		}

		glBindTexture(m_target, m_texture);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, m_format, width, height, 0, format, type, colors.Bytes());

		glBindTexture(m_target, 0);

		LogGLError();
	}
}
