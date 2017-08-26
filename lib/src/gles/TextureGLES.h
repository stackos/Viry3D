#pragma once

#include "Object.h"
#include "gles_include.h"

namespace Viry3D
{
	class TextureGLES : public Object
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

	private:
		void Create(GLenum format, GLenum type, void* pixels, bool mipmap);
		int GetMipCount();

		GLuint m_texture;
		GLint m_format;
	};
}