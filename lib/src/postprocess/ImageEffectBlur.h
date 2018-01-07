/*
* Viry3D
* Copyright 2014-2018 by Stack - stackos@qq.com
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

#include "ImageEffect.h"

namespace Viry3D
{
	class ImageEffectBlur: public ImageEffect
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
