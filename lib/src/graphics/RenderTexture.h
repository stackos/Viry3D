#pragma once

#include "Texture.h"
#include "RenderTextureFormat.h"
#include "DepthBuffer.h"
#include "container/List.h"

namespace Viry3D
{
	class RenderTexture : public Texture
	{
	public:
		static void Init();
		static void Deinit();
		static Ref<RenderTexture> Create(
			int width,
			int height,
			RenderTextureFormat::Enum format,
			DepthBuffer::Enum depth,
			FilterMode::Enum filter_mode);
		static Ref<RenderTexture> GetTemporary(int width,
			int height,
			RenderTextureFormat::Enum format,
			DepthBuffer::Enum depth,
			FilterMode::Enum filter_mode);
		static void ReleaseTemporary(Ref<RenderTexture> texture);

		RenderTextureFormat::Enum GetFormat() const { return m_format; }
		DepthBuffer::Enum GetDepth() const { return m_depth; }

	protected:
		void SetFormat(RenderTextureFormat::Enum format) { m_format = format; }
		void SetDepth(DepthBuffer::Enum depth) { m_depth = depth; }

	private:
		RenderTexture();

	private:
		struct Temporary
		{
			Ref<RenderTexture> texture;
			bool in_use;
			float used_time;
		};

		static Map<long long, List<Temporary>> m_temporarys;

		RenderTextureFormat::Enum m_format;
		DepthBuffer::Enum m_depth;
	};
}