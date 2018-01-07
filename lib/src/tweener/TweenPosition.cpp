/*
* Viry3D
* Copyright 2014-2018 by Stack - stackos@qq.com
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

		if (local_space)
		{
			GetTransform()->SetLocalPosition(pos);
		}
		else
		{
			GetTransform()->SetPosition(pos);
		}
	}
}
