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

#include "View.h"
#include "CanvaRenderer.h"
#include "Debug.h"
#include "memory/Memory.h"
#include "graphics/Texture.h"

namespace Viry3D
{
	View::View():
		m_canvas(nullptr),
		m_color(1, 1, 1, 1),
		m_alignment((int) ViewAlignment::HorizontalCenter | (int) ViewAlignment::VerticalCenter),
		m_pivot(0.5f, 0.5f),
		m_size(100, 100),
		m_offset(0, 0)
	{
	
	}

	View::~View()
	{
	
	}

	void View::OnAddToCanvas(CanvaRenderer* canvas)
	{
		assert(m_canvas == nullptr);
		m_canvas = canvas;
	}

	void View::OnRemoveFromCanvas(CanvaRenderer* canvas)
	{
		assert(m_canvas == canvas);
		m_canvas = nullptr;
	}

	void View::SetColor(const Color& color)
	{
		m_color = color;
		if (m_canvas)
		{
			m_canvas->MarkCanvasDirty();
		}
	}

	void View::SetAlignment(int alignment)
	{
		m_alignment = alignment;
		if (m_canvas)
		{
			m_canvas->MarkCanvasDirty();
		}
	}

	void View::SetPivot(const Vector2& pivot)
	{
		m_pivot = pivot;
		if (m_canvas)
		{
			m_canvas->MarkCanvasDirty();
		}
	}

	void View::SetSize(const Vector2& size)
	{
		m_size = size;
		if (m_canvas)
		{
			m_canvas->MarkCanvasDirty();
		}
	}

	void View::SetOffset(const Vector2& offset)
	{
		m_offset = offset;
		if (m_canvas)
		{
			m_canvas->MarkCanvasDirty();
		}
	}

    void View::UpdateLayout()
    {
    
    }

    void View::FillVertices(Vector<Vertex>& vertices, Vector<unsigned short>& indices, Vector<Ref<Texture>>& textures)
    {
        Vertex vs[4];
        Memory::Zero(&vs[0], sizeof(vs));
        vs[0].vertex = Vector3(-200.0f, 200.0f, 0);
        vs[1].vertex = Vector3(-200.0f, -200.0f, 0);
        vs[2].vertex = Vector3(200.0f, -200.0f, 0);
        vs[3].vertex = Vector3(200.0f, 200.0f, 0);
        vs[0].color = Color(1, 0, 0, 1);
        vs[1].color = Color(0, 1, 0, 1);
        vs[2].color = Color(0, 0, 1, 1);
        vs[3].color = Color(1, 1, 1, 1);
        vs[0].uv = Vector2(0, 0);
        vs[1].uv = Vector2(0, 1);
        vs[2].uv = Vector2(1, 1);
        vs[3].uv = Vector2(1, 0);

        vertices.AddRange({ vs[0], vs[1], vs[2], vs[3] });
        indices.AddRange({ 0, 1, 2, 0, 2, 3 });
		textures.Add(Texture::GetSharedWhiteTexture());
    }
}
