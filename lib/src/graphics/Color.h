#pragma once

namespace Viry3D
{
	struct Color
	{
		static Color Lerp(const Color &from, const Color &to, float t, bool clamp_01 = true);

		Color(float r = 0, float g = 0, float b = 0, float a = 0);
		bool operator ==(const Color &c) const;
		bool operator !=(const Color &c) const;
		Color operator *(const Color &c) const;
		Color &operator *=(const Color &c);
		Color operator *(float v) const;
		Color operator /(float v) const;

		float r;
		float g;
		float b;
		float a;
	};
}