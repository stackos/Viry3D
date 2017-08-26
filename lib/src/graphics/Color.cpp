#include "Color.h"
#include "math/Mathf.h"

namespace Viry3D
{
	Color::Color(float r, float g, float b, float a):
		r(r), g(g), b(b), a(a)
	{
	}

	bool Color::operator ==(const Color& c) const
	{
		return
			Mathf::FloatEqual(r, c.r) &&
			Mathf::FloatEqual(g, c.g) &&
			Mathf::FloatEqual(b, c.b) &&
			Mathf::FloatEqual(a, c.a);
	}

	bool Color::operator !=(const Color& c) const
	{
		return !(*this == c);
	}

	Color Color::Lerp(const Color& from, const Color& to, float t, bool clamp_01)
	{
		if(clamp_01)
		{
			t = Mathf::Clamp01(t);
		}

		return Color(
			Mathf::Lerp(from.r, to.r, t),
			Mathf::Lerp(from.g, to.g, t),
			Mathf::Lerp(from.b, to.b, t),
			Mathf::Lerp(from.a, to.a, t));
	}

	Color Color::operator *(const Color& c) const
	{
		return Color(r * c.r, g * c.g, b * c.b, a * c.a);
	}

	Color& Color::operator *=(const Color& c)
	{
		r *= c.r;
		g *= c.g;
		b *= c.b;
		a *= c.a;
		return *this;
	}

	Color Color::operator *(float v) const
	{
		return Color(r * v, g * v, b * v, a * v);
	}

	Color Color::operator /(float v) const
	{
		return *this * (1 / v);
	}
}