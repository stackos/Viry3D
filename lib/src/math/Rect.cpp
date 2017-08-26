#include "Rect.h"
#include "Mathf.h"

namespace Viry3D
{
	bool Rect::operator ==(const Rect& r) const
	{
		return Mathf::FloatEqual(x, r.x) &&
			Mathf::FloatEqual(y, r.y) &&
			Mathf::FloatEqual(width, r.width) &&
			Mathf::FloatEqual(height, r.height);
	}

	bool Rect::operator !=(const Rect& r) const
	{
		return !(*this == r);
	}
}