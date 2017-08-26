#include "ImageEffect.h"
#include "graphics/Material.h"
#include "graphics/Graphics.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(ImageEffect);

	void ImageEffect::DeepCopy(const Ref<Object>& source)
	{
		assert(!"can not copy this component");
	}

	void ImageEffect::OnRenderImage(Ref<RenderTexture> src, Ref<RenderTexture> dest)
	{
		Graphics::Blit(src, dest, m_material, -1);
	}
}