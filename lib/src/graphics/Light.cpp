#include "Light.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(Light);

	WeakRef<Light> Light::main;

	Light::~Light()
	{
	}

	void Light::DeepCopy(const Ref<Object>& source)
	{
		assert(!"not implemment!");
	}
}