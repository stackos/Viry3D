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

#pragma once

#include "string/String.h"
#include "container/Map.h"
#include "container/List.h"

namespace Viry3D
{
	struct ProfilerSample
	{
		float time;
		int call_count;
		float time_begin;
	};

	class Profiler
	{
	public:
		static void Reset();
		static void SampleBegin(const String& name);
		static void SampleEnd();
		static const Map<String, ProfilerSample>& GetSamples() { return m_samples; }
		static const ProfilerSample& GetSample(const String& name);

	private:
		static Map<String, ProfilerSample> m_samples;
		static List<ProfilerSample*> m_current_samples;
	};
}
