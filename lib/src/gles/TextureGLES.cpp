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
#include "Debug.h"
#include "math/Mathf.h"

namespace Viry3D
{
	TextureGLES::TextureGLES():
		m_texture(0),
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

		this->Create(format, type, NULL, false);
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

		this->Create(format, type, NULL, false);
	}

	void TextureGLES::CreateTexture2D()
	{
		auto texture = (Texture2D*) this;
		auto texture_format = texture->GetFormat();
		auto colors = texture->GetColors();
		auto mipmap = texture->IsMipmap();

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

		this->Create(format, type, colors.Bytes(), mipmap);
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

		glBindTexture(GL_TEXTURE_2D, m_texture);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, format, type, colors.Bytes());
		glBindTexture(GL_TEXTURE_2D, 0);

		LogGLError();
	}

	void TextureGLES::Create(GLenum format, GLenum type, void* pixels, bool mipmap)
	{
		LogGLError();

		auto texture = (Texture*) this;
		int width = texture->GetWidth();
		int height = texture->GetHeight();

		glGenTextures(1, &m_texture);
		glBindTexture(GL_TEXTURE_2D, m_texture);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, m_format, width, height, 0, format, type, pixels);

		if (mipmap)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		glBindTexture(GL_TEXTURE_2D, 0);

		UpdateSampler();

		LogGLError();
	}

    void TextureGLES::SetExternalTexture2D(void* texture)
    {
        m_texture = (GLuint) (size_t) texture;
        m_external = true;
    }
    
	int TextureGLES::GetMipCount()
	{
		int mip_count = 0;

		auto texture_render = dynamic_cast<RenderTexture*>(this);
		auto texture_2d = dynamic_cast<Texture2D*>(this);
		if (texture_render)
		{
			mip_count = 1;
		}
		else if (texture_2d)
		{
			int width = texture_2d->GetWidth();
			int height = texture_2d->GetHeight();
			bool mipmap = texture_2d->IsMipmap();

			if (mipmap)
			{
				mip_count = (int) floor(Mathf::Log2((float) Mathf::Max(width, height))) + 1;
			}
			else
			{
				mip_count = 1;
			}
		}

		return mip_count;
	}

	void TextureGLES::UpdateSampler()
	{
		LogGLError();

		bool mipmap = this->GetMipCount() > 1;

		glBindTexture(GL_TEXTURE_2D, m_texture);

		auto texture = (Texture*) this;
		GLuint filter_min = 0;
		GLuint filter_mag = 0;
		auto filter_mode = texture->GetFilterMode();
		switch (filter_mode)
		{
			case FilterMode::Point:
				if (mipmap)
				{
					filter_min = GL_NEAREST_MIPMAP_LINEAR;
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

		GLuint address_mode = 0;
		auto wrap_mode = texture->GetWrapMode();
		switch (wrap_mode)
		{
			case TextureWrapMode::Repeat:
				address_mode = GL_REPEAT;
				break;
			case TextureWrapMode::Clamp:
				address_mode = GL_CLAMP_TO_EDGE;
				break;
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, address_mode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, address_mode);

		if (m_format == GL_DEPTH_COMPONENT16 ||
			m_format == GL_DEPTH_COMPONENT24 ||
			m_format == GL_DEPTH24_STENCIL8 ||
			m_format == GL_DEPTH_COMPONENT32F)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_mag);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_min);
		}

		glBindTexture(GL_TEXTURE_2D, 0);
        
        LogGLError();
	}
}
