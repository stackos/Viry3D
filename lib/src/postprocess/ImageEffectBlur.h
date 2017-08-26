#pragma once

#include "ImageEffect.h"

namespace Viry3D
{
	class ImageEffectBlur : public ImageEffect
	{
		DECLARE_COM_CLASS(ImageEffectBlur, ImageEffect)
	public:
		virtual void Start();
		virtual void OnRenderImage(Ref<RenderTexture> src, Ref<RenderTexture> dest);
		int GetDownSample() const { return m_down_sample; }
		void SetDownSample(int value) { m_down_sample = value; }
		float GetBlurSize() const { return m_blur_size; }
		void SetBlurSize(float size) { m_blur_size = size; }
		int GetBlurIterations() const { return m_blur_iterations; }
		void SetBlurIterations(int iter) { m_blur_iterations = iter; }

	private:
		ImageEffectBlur();

	private:
		int m_down_sample;
		float m_blur_size;
		int m_blur_iterations;
	};
}