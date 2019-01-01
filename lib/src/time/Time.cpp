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

#include "Time.h"
#include "Debug.h"
#include <time.h>

#if VR_WINDOWS || VR_UWP
#include <windows.h>
#elif VR_IOS || VR_ANDROID || VR_MAC || VR_WASM
#include <sys/time.h>
#endif

namespace Viry3D
{
	long long Time::m_time_startup = 0;
	float Time::m_time_delta = 0;
	float Time::m_time_record = -1;
	int Time::m_frame_count = -1;
	int Time::m_frame_record;
	float Time::m_time = 0;
	int Time::m_fps;

	Date Time::GetDate()
	{
		Date date;

		long long t = GetTimeMS();
		date.milli_second = t % 1000;

		time_t ts = (time_t) (t / 1000);
		auto tm = localtime(&ts);

		date.year = tm->tm_year + 1900;
		date.month = tm->tm_mon + 1;
		date.day = tm->tm_mday;
		date.hour = tm->tm_hour;
		date.minute = tm->tm_min;
		date.second = tm->tm_sec;
		date.week_day = tm->tm_wday;

		return date;
	}

	long long Time::GetTimeMS()
	{
		long long t;

#if VR_WINDOWS || VR_UWP
		SYSTEMTIME sys_time;
		::GetLocalTime(&sys_time);

		struct tm tm;
		tm.tm_sec = sys_time.wSecond;
		tm.tm_min = sys_time.wMinute;
		tm.tm_hour = sys_time.wHour;
		tm.tm_mday = sys_time.wDay;
		tm.tm_mon = sys_time.wMonth - 1;
		tm.tm_year = sys_time.wYear - 1900;
		tm.tm_isdst = -1;

		t = mktime(&tm) * (long long) 1000 + sys_time.wMilliseconds;
#elif VR_IOS || VR_ANDROID || VR_MAC || VR_WASM
		struct timeval tv;
		gettimeofday(&tv, nullptr);
		t = tv.tv_sec;
		t *= 1000;
		t += tv.tv_usec / 1000;
#endif

		return t;
	}

	long long Time::GetUTCTimeMS()
	{
		long long t = GetTimeMS();
		int ms = t % 1000;
		time_t ts = (time_t) (t / 1000);
		auto tm = gmtime(&ts);
		t = mktime(tm) * (long long) 1000 + ms;

		return t;
	}

	float Time::GetTime()
	{
		return m_time;
	}

	float Time::GetRealTimeSinceStartup()
	{
		if (m_time_startup == 0)
		{
			m_time_startup = GetTimeMS();
		}

		long long time = GetTimeMS() - m_time_startup;

		return time / 1000.0f;
	}

	void Time::Update()
	{
		float time = Time::GetRealTimeSinceStartup();
		Time::m_time_delta = time - Time::m_time;
		Time::m_time = time;

		if (Time::m_time_record < 0)
		{
			Time::m_time_record = Time::GetRealTimeSinceStartup();
			Time::m_frame_record = Time::GetFrameCount();
		}

		int frame = Time::GetFrameCount();
		if (time - Time::m_time_record >= 1)
		{
			Time::m_fps = frame - Time::m_frame_record;
			Time::m_time_record = time;
			Time::m_frame_record = frame;
		}

        if (m_frame_count < 0)
        {
            m_frame_count = 0;
        }
        else
        {
            Time::m_frame_count++;
        }
	}
}
