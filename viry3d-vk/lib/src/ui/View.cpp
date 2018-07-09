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
#include "graphics/Camera.h"

namespace Viry3D
{
	View::View():
		m_canvas(nullptr),
        m_parent_view(nullptr),
		m_color(1, 1, 1, 1),
		m_alignment(ViewAlignment::HorizontalCenter | ViewAlignment::VerticalCenter),
		m_pivot(0.5f, 0.5f),
		m_size(100, 100),
		m_offset(0, 0),
        m_local_rotation(Quaternion::Identity()),
        m_local_scale(1, 1),
        m_rect(0, 0, 0, 0),
        m_rotation(Quaternion::Identity()),
        m_scale(1, 1)
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

    void View::AddSubview(const Ref<View>& view)
    {
        assert(view->m_parent_view == nullptr);
        view->m_parent_view = this;

        m_subviews.Add(view);

        if (m_canvas)
        {
            m_canvas->MarkCanvasDirty();
        }
    }

    void View::RemoveSubview(const Ref<View>& view)
    {
        assert(view->m_parent_view == this);
        view->m_parent_view = nullptr;
        
        m_subviews.Remove(view);

        if (m_canvas)
        {
            m_canvas->MarkCanvasDirty();
        }
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

    void View::SetLocalRotation(const Quaternion& rotation)
    {
        m_local_rotation = rotation;
        if (m_canvas)
        {
            m_canvas->MarkCanvasDirty();
        }
    }

    void View::SetLocalScale(const Vector2& scale)
    {
        m_local_scale = scale;
        if (m_canvas)
        {
            m_canvas->MarkCanvasDirty();
        }
    }

    void View::UpdateLayout()
    {
        Rect parent_rect;

        if (m_parent_view)
        {
            parent_rect = m_parent_view->GetRect();
        }
        else
        {
            parent_rect = Rect(0, 0, (float) m_canvas->GetCamera()->GetTargetWidth(), (float) m_canvas->GetCamera()->GetTargetHeight());
        }

        Vector2 local_pos;

        if (m_alignment & ViewAlignment::HorizontalLeft)
        {
            local_pos.x = 0;
        }
        else if (m_alignment & ViewAlignment::HorizontalCenter)
        {
            local_pos.x = parent_rect.width / 2;
        }
        else if (m_alignment & ViewAlignment::HorizontalRight)
        {
            local_pos.x = parent_rect.width;
        }

        if (m_alignment & ViewAlignment::VerticalTop)
        {
            local_pos.y = 0;
        }
        else if (m_alignment & ViewAlignment::VerticalCenter)
        {
            local_pos.y = parent_rect.height / 2;
        }
        else if (m_alignment & ViewAlignment::VerticalBottom)
        {
            local_pos.y = parent_rect.height;
        }

        local_pos += m_offset;

        m_rect.x = parent_rect.x + local_pos.x - m_pivot.x * m_size.x;
        m_rect.y = parent_rect.y + local_pos.y - m_pivot.y * m_size.y;
        m_rect.width = m_size.x;
        m_rect.height = m_size.y;

        if (m_parent_view)
        {
            m_rotation = m_parent_view->GetRotation() * m_local_rotation;

            Vector2 parent_scale = m_parent_view->GetScale();
            m_scale = Vector2(parent_scale.x * m_local_scale.x, parent_scale.y * m_local_scale.y);
        }
        else
        {
            m_rotation = m_local_rotation;
            m_scale = m_local_scale;
        }

        for (auto& i : m_subviews)
        {
            i->UpdateLayout();
        }
    }

    void View::FillSelfMesh(ViewMesh& mesh)
    {
        float canvas_width = (float) m_canvas->GetCamera()->GetTargetWidth();
        float canvas_height = (float) m_canvas->GetCamera()->GetTargetHeight();
        float x = -canvas_width / 2 + m_rect.x;
        float y = canvas_height / 2 - m_rect.y;
        Vector3 pivot_pos;
        pivot_pos.x = x + m_pivot.x * m_size.x;
        pivot_pos.y = y - m_pivot.y * m_size.y;
        pivot_pos.z = 0;
        Matrix4x4 matrix = Matrix4x4::Translation(pivot_pos) * Matrix4x4::Rotation(m_rotation) * Matrix4x4::Scaling(m_scale) * Matrix4x4::Translation(-pivot_pos);

        Vertex vs[4];
        Memory::Zero(&vs[0], sizeof(vs));
        vs[0].vertex = Vector3(x, y, 0);
        vs[1].vertex = Vector3(x, y - m_size.y, 0);
        vs[2].vertex = Vector3(x + m_size.x, y - m_size.y, 0);
        vs[3].vertex = Vector3(x + m_size.x, y, 0);
        vs[0].color = m_color;
        vs[1].color = m_color;
        vs[2].color = m_color;
        vs[3].color = m_color;
        vs[0].uv = Vector2(1.0f / 3, 1.0f / 3);
        vs[1].uv = Vector2(1.0f / 3, 2.0f / 3);
        vs[2].uv = Vector2(2.0f / 3, 2.0f / 3);
        vs[3].uv = Vector2(2.0f / 3, 1.0f / 3);

        for (int i = 0; i < 4; ++i)
        {
            vs[i].vertex = matrix.MultiplyPoint3x4(vs[i].vertex);
        }

        mesh.vertices.AddRange({ vs[0], vs[1], vs[2], vs[3] });
        mesh.indices.AddRange({ 0, 1, 2, 0, 2, 3 });
        mesh.texture = Texture::GetSharedWhiteTexture();
    }

    void View::FillMeshes(Vector<ViewMesh>& meshes)
    {
        ViewMesh mesh;
        this->FillSelfMesh(mesh);
        meshes.Add(mesh);

        for (auto& i : m_subviews)
        {
            i->FillMeshes(meshes);
        }
    }
}
