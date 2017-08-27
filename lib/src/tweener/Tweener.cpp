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

#include "Tweener.h"
#include "GameObject.h"
#include "time/Time.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(Tweener);

	Tweener::Tweener():
		curve(AnimationCurve::DefaultLinear()),
		duration(1.0f),
		delay(0),
		play_style(TweenerPlayStyle::Once),
		m_time_start(Time::GetTime()),
		m_time(0),
		m_reverse(false),
		m_finish(false)
	{
	}

	void Tweener::DeepCopy(const Ref<Object>& source)
	{
		Component::DeepCopy(source);

		auto src = RefCast<Tweener>(source);
		this->curve = src->curve;
		this->duration = src->duration;
		this->play_style = src->play_style;
		this->m_time_start = src->m_time_start;
		this->m_time = src->m_time;
		this->m_reverse = src->m_reverse;
		this->m_finish = src->m_finish;
	}

	void Tweener::Update()
	{
		float time = Time::GetTime();
		float t = -1;

		if (time - m_time_start >= delay && time - m_time_start - delay <= duration)
		{
			t = (time - m_time_start - delay) / duration;
		}
		else if (time - m_time_start - delay > duration)
		{
			if (play_style == TweenerPlayStyle::Once)
			{
				m_finish = true;
				if (m_time < 1)
				{
					t = 1;
				}
			}
			else if (play_style == TweenerPlayStyle::Loop)
			{
				m_time_start = time;
				t = 0;
			}
			else if (play_style == TweenerPlayStyle::PingPong)
			{
				m_time_start = time;
				t = 0;
				m_reverse = !m_reverse;
			}
		}

		if (t >= 0 && t <= 1)
		{
			m_time = t;

			float value;

			if (m_reverse)
			{
				value = curve.Evaluate(1 - t);
			}
			else
			{
				value = curve.Evaluate(t);
			}

			OnSetValue(value);
		}

		if (m_finish)
		{
			if (on_finish)
			{
				on_finish();
			}
			Component::Destroy(this->GetRef());
		}
	}
}
