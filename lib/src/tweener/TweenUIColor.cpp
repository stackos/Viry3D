#include "TweenUIColor.h"
#include "ui/UIView.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(TweenUIColor);

	TweenUIColor::TweenUIColor() :
		from(1, 1, 1, 1),
		to(1, 1, 1, 1)
	{
	}

	void TweenUIColor::DeepCopy(const Ref<Object>& source)
	{
		Tweener::DeepCopy(source);

		auto src = RefCast<TweenUIColor>(source);
		this->from = src->from;
		this->to = src->to;
	}

	void TweenUIColor::OnSetValue(float value)
	{
		auto color = Color::Lerp(from, to, value);
		auto views = this->GetGameObject()->GetComponentsInChildren<UIView>();
		for(auto i : views)
		{
			i->SetColor(color);
		}
	}
}