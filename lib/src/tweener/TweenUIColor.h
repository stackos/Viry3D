#pragma once

#include "Tweener.h"
#include "math/Vector3.h"
#include "graphics/Color.h"

namespace Viry3D
{
	class TweenUIColor : public Tweener
	{
		DECLARE_COM_CLASS(TweenUIColor, Tweener);

	protected:
		virtual void OnSetValue(float value);

	private:
		TweenUIColor();

	public:
		Color from;
		Color to;
	};
}