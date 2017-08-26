#pragma once

#include "Component.h"
#include "graphics/RenderTexture.h"

namespace Viry3D
{
	class Material;

	class ImageEffect : public Component
	{
		DECLARE_COM_CLASS(ImageEffect, Component)
	public:
		virtual void OnRenderImage(Ref<RenderTexture> src, Ref<RenderTexture> dest);

	protected:
		Ref<Material> m_material;
	};
}