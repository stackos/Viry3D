#pragma once

#include "Component.h"
#include "Color.h"

namespace Viry3D
{
	struct LightType
	{
		enum Enum
		{
			Spot,
			Directional,
			Point,
		};
	};

	class Light : public Component
	{
		DECLARE_COM_CLASS(Light, Component);

	public:
		virtual ~Light();

	private:
		Light():
			type(LightType::Directional),
			color(1, 1, 1, 1),
			intensity(1)
		{
		}

	public:
		static WeakRef<Light> main;
		LightType::Enum type;
		Color color;
		float intensity;
	};
}