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

#include "Profiler.h"
#include "time/Time.h"

namespace Viry3D
{
	Map<String, ProfilerSample> Profiler::m_samples;
	List<ProfilerSample*> Profiler::m_current_samples;

	void Profiler::Reset()
	{
		for (auto& i : m_samples)
		{
			i.second.call_count = 0;
			i.second.time = 0;
			i.second.time_begin = 0;
		}

		m_current_samples.Clear();
	}

	void Profiler::SampleBegin(const String& name)
	{
		ProfilerSample* sample;

		if (!m_samples.TryGet(name, &sample))
		{
			m_samples.Add(name, ProfilerSample());
			sample = &m_samples[name];

			sample->call_count = 0;
			sample->time = 0;
		}

		sample->time_begin = Time::GetRealTimeSinceStartup();

		m_current_samples.AddFirst(sample);
	}

	void Profiler::SampleEnd()
	{
		if (m_current_samples.Size() > 0)
		{
			auto sample = m_current_samples.First();
			m_current_samples.RemoveFirst();

			sample->time += Time::GetRealTimeSinceStartup() - sample->time_begin;
			sample->call_count++;
		}
	}

	const ProfilerSample& Profiler::GetSample(const String& name)
	{
		return m_samples[name];
	}
}
