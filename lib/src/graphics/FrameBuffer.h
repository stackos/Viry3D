#pragma once

#include "memory/Ref.h"

namespace Viry3D
{
	class RenderTexture;

	struct FrameBuffer
	{
		Ref<RenderTexture> color_texture;
		Ref<RenderTexture> depth_texture;
	};
}