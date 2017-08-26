#pragma once

#include "Component.h"

namespace Viry3D
{
	class Timer : public Component
	{
		DECLARE_COM_CLASS(Timer, Component);

	public:
		static WeakRef<Timer> CreateTimer(float duration, bool loop = false);
		void Stop();

	protected:
		Timer();
		virtual void Update();

	public:
		std::function<void()> on_tick;
		int tick_count;

	protected:
		float m_duration;
		bool m_loop;
		float m_time_start;
	};
}