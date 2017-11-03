/*
* Viry3D
* Copyright 2014-2017 by Stack - stackos@qq.com
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

#include "Display.h"
#include "memory/Ref.h"
#include "container/Vector.h"
#include "math/Rect.h"

namespace Viry3D
{
	enum class CullFace
	{
		NoSet,
		Off,
		Back,
		Front,
	};

	class Display;
	class Material;
	class Texture;
	class Mesh;
	class RenderTexture;
	class RenderPass;
	struct Matrix4x4;
	class DescriptorSet;
	class UniformBuffer;

	class Graphics
	{
	public:
		static void Init(int width, int height, int fps);
		static void OnResize(int width, int height);
		static void OnPause();
		static void OnResume();
		static void Deinit();
		static Display* GetDisplay();
		static void Render();

		//
		//	rect in screen range (0, 0, 1, 1)
		//	0,1		1,1
		//   ©°-------©´
		//   |		 |
		//   ©¸-------©¼
		//	0,0		1,0
		static void DrawQuad(const Rect* rect, const Ref<Texture>& texture, bool reverse_uv_y = false);
		static void DrawQuad(const Rect* rect, const Ref<Material>& material, int pass, bool reverse_uv_y = false);
		static void Blit(const Ref<RenderTexture>& src, const Ref<RenderTexture>& dest, const Ref<Material>& material = Ref<Material>(), int pass = 0, const Rect* rect = NULL);
		
		static CullFace GetGlobalCullFace() { return m_global_cull_face; }
		static void SetGlobalCullFace(CullFace cull_face) { m_global_cull_face = cull_face; }

	public:
		static int draw_call;

	private:
		static Ref<Display> m_display;
		static Ref<Mesh> m_blit_mesh;
		static Vector<Ref<Material>> m_blit_materials;
		static Vector<Ref<RenderPass>> m_blit_render_passes;
		static Ref<DescriptorSet> m_blit_descriptor_set;
		static Ref<UniformBuffer> m_blit_descriptor_set_buffer;
		static CullFace m_global_cull_face;
	};
}
