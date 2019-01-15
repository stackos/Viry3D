/*
* Viry3D
* Copyright 2014-2019 by Stack - stackos@qq.com
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
#include "CanvasRenderer.h"
#include "Debug.h"
#include "memory/Memory.h"
#include "graphics/Camera.h"

namespace Viry3D
{
	View::View():
		m_canvas(nullptr),
        m_parent_view(nullptr),
		m_color(1, 1, 1, 1),
		m_alignment(ViewAlignment::HCenter | ViewAlignment::VCenter),
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

	void View::OnAddToCanvas(CanvasRenderer* canvas)
	{
		assert(m_canvas == nullptr);
		m_canvas = canvas;
	}

	void View::OnRemoveFromCanvas(CanvasRenderer* canvas)
	{
		assert(m_canvas == canvas);
		m_canvas = nullptr;
	}

    CanvasRenderer* View::GetCanvas() const
    {
        if (m_canvas)
        {
            return m_canvas; 
        }
        else if (m_parent_view)
        {
            return m_parent_view->GetCanvas();
        }

        return nullptr;
    }

    void View::MarkCanvasDirty() const
    {
        CanvasRenderer* canvas = this->GetCanvas();
        if (canvas)
        {
            canvas->MarkCanvasDirty();
        }
    }

    void View::AddSubview(const Ref<View>& view)
    {
        assert(view->m_parent_view == nullptr);
        view->m_parent_view = this;

        m_subviews.Add(view);

        this->MarkCanvasDirty();
    }

    void View::RemoveSubview(const Ref<View>& view)
    {
        assert(view->m_parent_view == this);
        view->m_parent_view = nullptr;
        
        m_subviews.Remove(view);

        this->MarkCanvasDirty();
    }

    void View::ClearSubviews()
    {
        Vector<Ref<View>> subviews;
        for (int i = 0; i < this->GetSubviewCount(); ++i)
        {
            subviews.Add(this->GetSubview(i));
        }
        for (int i = 0; i < subviews.Size(); ++i)
        {
            this->RemoveSubview(subviews[i]);
        }
    }

	void View::SetColor(const Color& color)
	{
		m_color = color;
        this->MarkCanvasDirty();
	}

	void View::SetAlignment(int alignment)
	{
		m_alignment = alignment;
        this->MarkCanvasDirty();
	}

	void View::SetPivot(const Vector2& pivot)
	{
		m_pivot = pivot;
        this->MarkCanvasDirty();
	}

	void View::SetSize(const Vector2i& size)
	{
		m_size = size;
        this->MarkCanvasDirty();
	}

    Vector2i View::GetCalculateddSize()
    {
        Vector2i size = m_size;
        
        if (size.x == VIEW_SIZE_FILL_PARENT ||
            size.y == VIEW_SIZE_FILL_PARENT)
        {
            Vector2i parent_size;

            if (m_parent_view)
            {
                parent_size = m_parent_view->GetCalculateddSize();
            }
            else
            {
                parent_size.x = m_canvas->GetCamera()->GetTargetWidth();
                parent_size.y = m_canvas->GetCamera()->GetTargetHeight();
            }

            if (size.x == VIEW_SIZE_FILL_PARENT)
            {
                size.x = parent_size.x;
            }

            if (size.y == VIEW_SIZE_FILL_PARENT)
            {
                size.y = parent_size.y;
            }
        }

        return size;
    }

	void View::SetOffset(const Vector2i& offset)
	{
		m_offset = offset;
        this->MarkCanvasDirty();
	}

    void View::SetLocalRotation(const Quaternion& rotation)
    {
        m_local_rotation = rotation;
        this->MarkCanvasDirty();
    }

    void View::SetLocalScale(const Vector2& scale)
    {
        m_local_scale = scale;
        this->MarkCanvasDirty();
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

        Vector2i local_pos;

        if (m_alignment & ViewAlignment::Left)
        {
            local_pos.x = 0;
        }
        else if (m_alignment & ViewAlignment::HCenter)
        {
            local_pos.x = (int) (parent_rect.width / 2);
        }
        else if (m_alignment & ViewAlignment::Right)
        {
            local_pos.x = (int) parent_rect.width;
        }

        if (m_alignment & ViewAlignment::Top)
        {
            local_pos.y = 0;
        }
        else if (m_alignment & ViewAlignment::VCenter)
        {
            local_pos.y = (int) (parent_rect.height / 2);
        }
        else if (m_alignment & ViewAlignment::Bottom)
        {
            local_pos.y = (int) parent_rect.height;
        }

        local_pos += m_offset;

        Vector2i size = this->GetSize();
        if (size.x == VIEW_SIZE_FILL_PARENT)
        {
            size.x = (int) parent_rect.width;
        }
        if (size.y == VIEW_SIZE_FILL_PARENT)
        {
            size.y = (int) parent_rect.height;
        }

        m_rect.x = parent_rect.x + local_pos.x - Mathf::Round(m_pivot.x * size.x);
        m_rect.y = parent_rect.y + local_pos.y - Mathf::Round(m_pivot.y * size.y);
        m_rect.width = (float) size.x;
        m_rect.height = (float) size.y;

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

    void View::OnResize(int width, int height)
    {
        for (auto& i : m_subviews)
        {
            i->UpdateLayout();
        }
    }

    void View::ComputeVerticesRectAndMatrix(Rect& rect, Matrix4x4& matrix)
    {
        int x = (int) m_rect.x;
        int y = (int) -m_rect.y;

        rect = Rect((float) x, (float) y, m_rect.width, m_rect.height);

        Vector3 pivot_pos;
        pivot_pos.x = x + Mathf::Round(m_pivot.x * m_rect.width);
        pivot_pos.y = y - Mathf::Round(m_pivot.y * m_rect.height);
        pivot_pos.z = 0;

        matrix = Matrix4x4::Translation(pivot_pos) * Matrix4x4::Rotation(m_rotation) * Matrix4x4::Scaling(m_scale) * Matrix4x4::Translation(-pivot_pos);
    }

    void View::FillSelfMeshes(Vector<ViewMesh>& meshes)
    {
        Rect rect;
        Matrix4x4 matrix;
        this->ComputeVerticesRectAndMatrix(rect, matrix);

        Vertex vs[4];
        Memory::Zero(&vs[0], sizeof(vs));
        vs[0].vertex = Vector3(rect.x, rect.y, 0);
        vs[1].vertex = Vector3(rect.x, rect.y - rect.height, 0);
        vs[2].vertex = Vector3(rect.x + rect.width, rect.y - rect.height, 0);
        vs[3].vertex = Vector3(rect.x + rect.width, rect.y, 0);
        vs[0].color = m_color;
        vs[1].color = m_color;
        vs[2].color = m_color;
        vs[3].color = m_color;
        vs[0].uv = Vector2(0, 0);
        vs[1].uv = Vector2(0, 1);
        vs[2].uv = Vector2(1, 1);
        vs[3].uv = Vector2(1, 0);

        for (int i = 0; i < 4; ++i)
        {
            vs[i].vertex = matrix.MultiplyPoint3x4(vs[i].vertex);
        }

        ViewMesh mesh;
        mesh.vertices.AddRange({ vs[0], vs[1], vs[2], vs[3] });
        mesh.indices.AddRange({ 0, 1, 2, 0, 2, 3 });
        mesh.view = this;
        mesh.base_view = true;
        meshes.Add(mesh);
    }

    void View::FillMeshes(Vector<ViewMesh>& meshes)
    {
        this->FillSelfMeshes(meshes);

        for (auto& i : m_subviews)
        {
            i->FillMeshes(meshes);
        }
    }

    bool View::OnTouchDownInside(const Vector2i& pos) const
    {
        if (m_on_touch_down_inside)
        {
            return m_on_touch_down_inside(pos);
        }
        return false;
    }

    bool View::OnTouchMoveInside(const Vector2i& pos) const
    {
        if (m_on_touch_move_inside)
        {
            return m_on_touch_move_inside(pos);
        }
        return false;
    }
    
    bool View::OnTouchUpInside(const Vector2i& pos) const
    {
        if (m_on_touch_up_inside)
        {
            return m_on_touch_up_inside(pos);
        }
        return false;
    }

    bool View::OnTouchUpOutside(const Vector2i& pos) const
    {
        if (m_on_touch_up_outside)
        {
            return m_on_touch_up_outside(pos);
        }
        return false;
    }

    bool View::OnTouchDrag(const Vector2i& pos) const
    {
        if (m_on_touch_drag)
        {
            return m_on_touch_drag(pos);
        }
        return false;
    }
}
