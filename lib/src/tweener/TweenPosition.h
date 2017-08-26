#pragma once

#include "Tweener.h"
#include "math/Vector3.h"

namespace Viry3D
{
	class TweenPosition : public Tweener
	{
		DECLARE_COM_CLASS(TweenPosition, Tweener);

	protected:
		virtual void OnSetValue(float value);

	private:
		TweenPosition();

	public:
		Vector3 from;
		Vector3 to;
		bool local_space;
	};
}