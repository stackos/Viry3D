#include "TweenPosition.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(TweenPosition);

	TweenPosition::TweenPosition():
		from(0, 0, 0),
		to(0, 0, 0),
		local_space(true)
	{
	}

	void TweenPosition::DeepCopy(const Ref<Object>& source)
	{
		Tweener::DeepCopy(source);

		auto src = RefCast<TweenPosition>(source);
		this->from = src->from;
		this->to = src->to;
		this->local_space = src->local_space;
	}

	void TweenPosition::OnSetValue(float value)
	{
		auto pos = Vector3::Lerp(from, to, value);

		if(local_space)
		{
			GetTransform()->SetLocalPosition(pos);
		}
		else
		{
			GetTransform()->SetPosition(pos);
		}
	}
}