#include "Timer.h"
#include "Time.h"
#include "GameObject.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(Timer);

	WeakRef<Timer> Timer::CreateTimer(float duration, bool loop)
	{
		auto timer = GameObject::Create("Timer")->AddComponent<Timer>();
		timer->m_duration = duration;
		timer->m_loop = loop;

		return timer;
	}

	void Timer::Stop()
	{
		GameObject::Destroy(this->GetGameObject());
	}

	Timer::Timer() :
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

		if(time - m_time_start >= m_duration)
		{
			if(on_tick)
			{
				tick_count++;
				on_tick();
			}

			if(m_loop)
			{
				m_time_start = time;
			}
			else
			{
				Stop();
			}
		}
	}
}