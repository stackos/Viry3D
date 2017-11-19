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

#include "Timer.h"
#include "Time.h"
#include "GameObject.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(Timer);

	Ref<Timer> Timer::Start(float duration, bool loop)
	{
		auto timer = GameObject::Create("Timer")->AddComponent<Timer>();
		timer->m_duration = duration;
		timer->m_loop = loop;

		return timer;
	}

	void Timer::Stop(const Ref<Timer>& timer)
	{
        timer->Stop();
	}
    
    void Timer::Stop()
    {
        this->on_tick = NULL;
        GameObject::Destroy(this->GetGameObject());
    }

	Timer::Timer():
		tick_count(0),
		m_duration(1.0f),
		m_loop(false),
		m_time_start(Time::GetTime())
	{
	}

	void Timer::DeepCopy(const Ref<Object>& source)
	{
		assert(!"can not copy this component");
	}

	void Timer::Update()
	{
		float time = Time::GetTime();

		if (time - m_time_start >= m_duration)
		{
			if (on_tick)
			{
				tick_count++;
				on_tick(this);
			}

			if (m_loop)
			{
				m_time_start = time;
			}
			else
			{
				Timer::Stop(RefCast<Timer>(this->GetRef()));
			}
		}
	}
}
