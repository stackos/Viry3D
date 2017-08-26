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