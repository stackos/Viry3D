#include "Time.h"
#include "Debug.h"
#include <time.h>

#if VR_WINDOWS
#include <windows.h>
#elif VR_IOS || VR_ANDROID
#include <sys/time.h>
#endif

namespace Viry3D
{
	long long Time::m_time_startup = 0;
	float Time::m_time_delta = 0;
	float Time::m_time_record = -1;
	int Time::m_frame_count = 0;
	int Time::m_frame_record;
	float Time::m_time = 0;
	int Time::m_fps;

	Date Time::GetDate()
	{
		Date date;

		long long t = GetTimeMS();
		date.milli_second = t % 1000;

		time_t ts = t / 1000;
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

#if VR_WINDOWS
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
#elif VR_IOS || VR_ANDROID
		struct timeval tv;
		gettimeofday(&tv, NULL);
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
		time_t ts = t / 1000;
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
		if(m_time_startup == 0)
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

		if(Time::m_time_record < 0)
		{
			Time::m_time_record = Time::GetRealTimeSinceStartup();
			Time::m_frame_record = Time::GetFrameCount();
		}

		int frame = Time::GetFrameCount();
		if(time - Time::m_time_record >= 1)
		{
			Time::m_fps = frame - Time::m_frame_record;
			Time::m_time_record = time;
			Time::m_frame_record = frame;

			//Log("fps:%d", Time::GetFPS());
		}

		Time::m_frame_count++;
	}
}
