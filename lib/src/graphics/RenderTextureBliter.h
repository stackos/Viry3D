#pragma once

#include "Component.h"
#include "FrameBuffer.h"

namespace Viry3D
{
	class RenderTextureBliter : public Component
	{
		DECLARE_COM_CLASS(RenderTextureBliter, Component)
	public:
		Ref<FrameBuffer> rt;

	protected:
		virtual void Start();
		virtual void OnPostRender();
	};
}