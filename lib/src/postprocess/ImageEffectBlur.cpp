#include "ImageEffectBlur.h"
#include "graphics/Material.h"
#include "graphics/Graphics.h"
#include "graphics/RenderTexture.h"
#include "graphics/FrameBuffer.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(ImageEffectBlur);

	ImageEffectBlur::ImageEffectBlur()
	{
		this->SetDownSample(2);
		this->SetBlurSize(4);
		this->SetBlurIterations(3);
	}

	void ImageEffectBlur::DeepCopy(const Ref<Object>& source)
	{
		assert(!"can not copy this component");
	}

	void ImageEffectBlur::Start()
	{
		m_material = Material::Create("ImageEffect/Blur");
	}

	void ImageEffectBlur::OnRenderImage(Ref<RenderTexture> src, Ref<RenderTexture> dest)
	{
		if(src->GetFilterMode() != FilterMode::Bilinear)
		{
			src->SetFilterMode(FilterMode::Bilinear);
			src->UpdateSampler();
		}

		int downsample = this->GetDownSample();
		int rt_w = src->GetWidth() >> downsample;
		int rt_h = src->GetHeight() >> downsample;

		auto rt = RenderTexture::GetTemporary(rt_w, rt_h, src->GetFormat(), DepthBuffer::Depth_0, FilterMode::Bilinear);

		m_material->SetMainTexTexelSize(src);
		Graphics::Blit(src, rt, m_material, 0);

		float blur_size = this->GetBlurSize();
		float width_mod = 1.0f / (1.0f * (1 << downsample));

		m_material->SetMainTexTexelSize(rt);

		int blur_iter = this->GetBlurIterations();
		for(int i = 0; i < blur_iter; i++)
		{
			float offset = i * 1.0f;
			m_material->SetVector("_Parameter", Vector4(blur_size * width_mod + offset, -blur_size * width_mod - offset, 0, 0));

			auto rt2 = RenderTexture::GetTemporary(rt_w, rt_h, src->GetFormat(), DepthBuffer::Depth_0, FilterMode::Bilinear);
			Graphics::Blit(rt, rt2, m_material, 1);
			RenderTexture::ReleaseTemporary(rt);
			rt = rt2;

			rt2 = RenderTexture::GetTemporary(rt_w, rt_h, src->GetFormat(), DepthBuffer::Depth_0, FilterMode::Bilinear);
			Graphics::Blit(rt, rt2, m_material, 2);
			RenderTexture::ReleaseTemporary(rt);
			rt = rt2;
		}

		Graphics::Blit(rt, dest);

		RenderTexture::ReleaseTemporary(rt);
	}
}