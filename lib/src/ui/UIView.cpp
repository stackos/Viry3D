#include "UIView.h"
#include "UICanvasRenderer.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(UIView);

	UIView::UIView() :
		m_color(1, 1, 1, 1)
	{
	}

	void UIView::DeepCopy(const Ref<Object>& source)
	{
		Component::DeepCopy(source);

		auto src = RefCast<UIView>(source);
		m_color = src->m_color;
		m_anchor_min = src->m_anchor_min;
		m_anchor_max = src->m_anchor_max;
		m_offset_min = src->m_offset_min;
		m_offset_max = src->m_offset_max;
		m_pivot = src->m_pivot;
		m_dirty = true;
	}

	void UIView::SetAnchors(const Vector2& min, const Vector2& max)
	{
		UIRect::SetAnchors(min, max);

		if(m_dirty)
		{
			MarkRendererDirty();
		}
	}

	void UIView::SetOffsets(const Vector2& min, const Vector2& max)
	{
		UIRect::SetOffsets(min, max);

		if(m_dirty)
		{
			MarkRendererDirty();
		}
	}

	void UIView::SetPivot(const Vector2& pivot)
	{
		UIRect::SetPivot(pivot);

		if(m_dirty)
		{
			MarkRendererDirty();
		}
	}

	void UIView::SetColor(const Color& color)
	{
		if(m_color != color)
		{
			m_color = color;
			m_dirty = true;
			MarkRendererDirty();
		}
	}

	void UIView::OnTranformChanged()
	{
		m_dirty = true;
		MarkRendererDirty();
	}

	void UIView::SetRenderer(const Ref<UICanvasRenderer>& renderer)
	{
		m_renderer = renderer;
	}

	void UIView::MarkRendererDirty()
	{
		if(!m_renderer.expired())
		{
			m_renderer.lock()->MarkDirty();
			m_renderer.reset();
		}
	}

	Matrix4x4 UIView::GetVertexMatrix()
	{
		auto local_position = this->GetTransform()->GetLocalPosition();
		auto local_rotation = this->GetTransform()->GetLocalRotation();
		auto local_scale = this->GetTransform()->GetLocalScale();

		auto mat_local = Matrix4x4::TRS(local_position, local_rotation, local_scale);
		auto mat_parent_to_world = this->GetTransform()->GetParent().lock()->GetLocalToWorldMatrix();
		auto mat_world_to_canvas = m_renderer.lock()->GetTransform()->GetWorldToLocalMatrix();
		auto mat = mat_world_to_canvas * mat_parent_to_world * mat_local;

		return mat;
	}

	Vector<Vector3> UIView::GetBoundsVertices()
	{
		Vector<Vector3> vertices;

		Vector2 size = this->GetSize();
		Vector2 min = Vector2(-m_pivot.x * size.x, -m_pivot.y * size.y);
		Vector2 max = Vector2((1 - m_pivot.x) * size.x, (1 - m_pivot.y) * size.y);
		vertices.Add(Vector3(min.x, min.y, 0));
		vertices.Add(Vector3(max.x, min.y, 0));
		vertices.Add(Vector3(max.x, max.y, 0));
		vertices.Add(Vector3(min.x, max.y, 0));

		auto mat = GetVertexMatrix();
		for(int i = 0; i < 4; i++)
		{
			auto v = vertices[i];
			v.x = floor(v.x);
			v.y = floor(v.y);

			vertices[i] = mat.MultiplyPoint3x4(v);
		}

		return vertices;
	}

	void UIView::FillVertices(Vector<Vector3>& vertices, Vector<Vector2>& uv, Vector<Color>& colors, Vector<unsigned short>& indices)
	{
		auto vertex_array = GetBoundsVertices();
		vertices.AddRange(&vertex_array[0], 4);

		uv.Add(Vector2(0, 1));
		uv.Add(Vector2(1, 1));
		uv.Add(Vector2(1, 0));
		uv.Add(Vector2(0, 0));

		colors.Add(m_color);
		colors.Add(m_color);
		colors.Add(m_color);
		colors.Add(m_color);

		int index_begin = vertices.Size() - 4;
		indices.Add(index_begin + 0);
		indices.Add(index_begin + 1);
		indices.Add(index_begin + 2);
		indices.Add(index_begin + 0);
		indices.Add(index_begin + 2);
		indices.Add(index_begin + 3);
	}

	void UIView::FillMaterial(Ref<Material>& mat)
	{
	}
}