#include "RenderTextureBliter.h"
#include "Graphics.h"
#include "GameObject.h"
#include "Camera.h"
#include "RenderTexture.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(RenderTextureBliter);

	void RenderTextureBliter::DeepCopy(const Ref<Object>& source)
	{
	}

	void RenderTextureBliter::Start()
	{
		GetGameObject()->GetComponent<Camera>()->SetAlwaysRebuildCmd();
	}

	void RenderTextureBliter::OnPostRender()
	{
#if VR_GLES
		bool reverse_uv_y = true;
#else
		bool reverse_uv_y = false;
#endif

		Rect rect(0, 0, 1, 1);
		Graphics::DrawQuad(&rect, rt->color_texture, reverse_uv_y);
	}
}