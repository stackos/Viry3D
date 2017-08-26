#pragma once

#include "math/Vector2.h"
#include "math/Rect.h"

namespace Viry3D
{
	class UIRect
	{
	public:
		virtual void SetAnchors(const Vector2& min, const Vector2& max);
		virtual void SetOffsets(const Vector2& min, const Vector2& max);
		virtual void SetPivot(const Vector2& pivot);
		virtual void SetSize(const Vector2& size);
		const Vector2& GetAnchorMin() const { return m_anchor_min; }
		const Vector2& GetAnchorMax() const { return m_anchor_max; }
		const Vector2& GetOffsetMin() const { return m_offset_min; }
		const Vector2& GetOffsetMax() const { return m_offset_max; }
		const Vector2& GetPivot() const { return m_pivot; }
		void OnAnchor();

	protected:
		UIRect();
		Ref<UIRect> GetParentRect() const;
		Vector2 GetSize() const;

		Vector2 m_anchor_min;
		Vector2 m_anchor_max;
		Vector2 m_offset_min;
		Vector2 m_offset_max;
		Vector2 m_pivot;
		bool m_dirty;
	};
}