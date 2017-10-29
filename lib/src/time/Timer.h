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

#pragma once

#include "Component.h"
#include "Action.h"

namespace Viry3D
{
	class Timer: public Component
	{
		DECLARE_COM_CLASS(Timer, Component);

	public:
		static WeakRef<Timer> Create(float duration, bool loop = false);
		void Stop();

	protected:
		Timer();
		virtual void Update();

	public:
		Action on_tick;
		int tick_count;

	protected:
		float m_duration;
		bool m_loop;
		float m_time_start;
	};
}
