/*
* Viry3D
* Copyright 2014-2019 by Stack - stackos@qq.com
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

namespace Viry3D
{
	struct Date
	{
		int year;
		int month;
		int day;
		int hour;
		int minute;
		int second;
		int milli_second;
		int week_day;
	};

	class Time
	{
		friend class Application;
		friend class Camera;
		friend class Renderer;

	public:
		static int GetFrameCount() { return m_frame_count; }
		//	calls have same time value in same frame
		static float GetTime();
		static float GetRealTimeSinceStartup();
		static float GetDeltaTime() { return m_time_delta; }
		//	local time in ms since 1970
		static long long GetTimeMS();
		//	utc time in ms since 1970
		static long long GetUTCTimeMS();
		//	local date
		static Date GetDate();
		static int GetFPS() { return m_fps; }
		static void Update();

	private:
		static long long m_time_startup;
		static float m_time_delta;
		static float m_time_record;
		static float m_time;
		static int m_frame_count;
		static int m_frame_record;
		static int m_fps;
	};
}
