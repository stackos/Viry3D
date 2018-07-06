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

#include "graphics/Renderer.h"
#include "container/Vector.h"

namespace Viry3D
{
	class View;
	class Mesh;

	class CanvaRenderer : public Renderer
	{
	public:
		CanvaRenderer();
		virtual ~CanvaRenderer();
		virtual Ref<BufferObject> GetVertexBuffer() const;
		virtual Ref<BufferObject> GetIndexBuffer() const;
        virtual Ref<BufferObject> GetDrawBuffer() const { return m_draw_buffer; }
		virtual void Update();
        virtual void OnResize(int width, int height);
		void AddView(const Ref<View>& view);
		void RemoveView(const Ref<View>& view);
		void MarkCanvasDirty();

	private:
        void CreateAtlasTexture();
        void CreateMaterial();
		void UpdateCanvas();
        void UpdateProjectionMatrix();

	private:
		Vector<Ref<View>> m_views;
		bool m_canvas_dirty;
		Ref<Mesh> m_mesh;
        Ref<Texture> m_atlas;
        Ref<BufferObject> m_draw_buffer;
	};
}
