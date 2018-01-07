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

#include "UIRect.h"
#include "UIView.h"
#include "UICanvasRenderer.h"
#include "graphics/Camera.h"

namespace Viry3D
{
	UIRect::UIRect():
		m_anchor_min(0.5f, 0.5f),
		m_anchor_max(0.5f, 0.5f),
		m_offset_min(-50.0f, -50.0f),
		m_offset_max(50.0f, 50.0f),
		m_pivot(0.5f, 0.5f),
		m_dirty(true)
	{
	}

	void UIRect::SetAnchors(const Vector2& min, const Vector2& max)
	{
		if (m_anchor_min != min || m_anchor_max != max)
		{
			m_anchor_min = min;
			m_anchor_max = max;
			m_dirty = true;
		}
	}

	void UIRect::SetOffsets(const Vector2& min, const Vector2& max)
	{
		if (m_offset_min != min || m_offset_max != max)
		{
			m_offset_min = min;
			m_offset_max = max;
			m_dirty = true;
		}
	}

	void UIRect::SetPivot(const Vector2& pivot)
	{
		if (m_pivot != pivot)
		{
			m_pivot = pivot;
			m_dirty = true;
		}
	}

	void UIRect::SetSize(const Vector2& size)
	{
		if (GetSize() != size)
		{
			auto parent = this->GetParentRect();
			if (parent)
			{
				auto c = dynamic_cast<const Component*>(this);
				auto pos = c->GetTransform()->GetLocalPosition();
				auto min_x = pos.x - m_pivot.x * size.x;
				auto min_y = pos.y - m_pivot.y * size.y;
				auto max_x = pos.x + (1.0f - m_pivot.x) * size.x;
				auto max_y = pos.y + (1.0f - m_pivot.y) * size.y;

				auto parent_size = parent->GetSize();
				auto parent_x = parent->m_pivot.x * parent_size.x;
				auto parent_y = parent->m_pivot.y * parent_size.y;

				m_offset_min.x = min_x + parent_x - m_anchor_min.x * parent_size.x;
				m_offset_min.y = min_y + parent_y - m_anchor_min.y * parent_size.y;
				m_offset_max.x = max_x + parent_x - m_anchor_max.x * parent_size.x;
				m_offset_max.y = max_y + parent_y - m_anchor_max.y * parent_size.y;
			}
			else
			{
				m_offset_min = Vector2(size.x * -m_pivot.x, size.y * -m_pivot.y);
				m_offset_max = Vector2(size.x * (1.0f - m_pivot.x), size.y * (1.0f - m_pivot.y));
			}

			m_dirty = true;
		}
	}

	Vector2 UIRect::GetSize() const
	{
		Vector2 size;

		auto parent = this->GetParentRect();
		if (parent)
		{
			auto parent_size = parent->GetSize();
			auto min_x = m_anchor_min.x * parent_size.x + m_offset_min.x;
			auto min_y = m_anchor_min.y * parent_size.y + m_offset_min.y;
			auto max_x = m_anchor_max.x * parent_size.x + m_offset_max.x;
			auto max_y = m_anchor_max.y * parent_size.y + m_offset_max.y;

			size = Vector2(max_x - min_x, max_y - min_y);
		}
		else
		{
			size = m_offset_max - m_offset_min;
		}

		return size;
	}

	Ref<UIRect> UIRect::GetParentRect() const
	{
		Ref<UIRect> rect;

		auto c = dynamic_cast<const Component*>(this);
		auto parent = c->GetTransform()->GetParent();
		if (!parent.expired())
		{
			auto r = parent.lock()->GetGameObject()->GetComponent<UIRect>();

			if (r)
			{
				rect = r;
			}
		}

		return rect;
	}

	void UIRect::OnAnchor()
	{
		auto com = dynamic_cast<const Component*>(this);

		auto parent = this->GetParentRect();
		if (parent)
		{
			auto parent_size = parent->GetSize();
			auto min_x = m_anchor_min.x * parent_size.x + m_offset_min.x;
			auto min_y = m_anchor_min.y * parent_size.y + m_offset_min.y;
			auto max_x = m_anchor_max.x * parent_size.x + m_offset_max.x;
			auto max_y = m_anchor_max.y * parent_size.y + m_offset_max.y;

			auto x = min_x + m_pivot.x * (max_x - min_x);
			auto y = min_y + m_pivot.y * (max_y - min_y);
			auto parent_x = parent_size.x * parent->m_pivot.x;
			auto parent_y = parent_size.y * parent->m_pivot.y;

			auto pos = Vector2(x - parent_x, y - parent_y);
			com->GetTransform()->SetLocalPosition(pos);
		}

		int child_count = com->GetTransform()->GetChildCount();
		for (int i = 0; i < child_count; i++)
		{
			auto child = com->GetTransform()->GetChild(i);
			auto view = child->GetGameObject()->GetComponent<UIView>();
			if (view)
			{
				view->OnAnchor();
			}
			
			auto canvas = child->GetGameObject()->GetComponent<UICanvasRenderer>();
			if (canvas)
			{
				canvas->OnAnchor();
			}
		}
	}
}
