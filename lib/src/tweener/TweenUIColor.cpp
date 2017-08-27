/*
* Viry3D
* Copyright 2014-2017 by Stack - stackos@qq.com
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "TweenUIColor.h"
#include "ui/UIView.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(TweenUIColor);

	TweenUIColor::TweenUIColor():
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
		for (auto i : views)
		{
			i->SetColor(color);
		}
	}
}
