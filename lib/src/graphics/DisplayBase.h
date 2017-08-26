#pragma once

#include "string/String.h"

namespace Viry3D
{
	class DisplayBase
	{
	public:
		void Init(int width, int height, int fps);
		void ProcessSystemEvents();
		int GetWidth() const { return m_width; }
		int GetHeight() const { return m_height; }
		int GetPreferredFPS() const { return m_fps; }
		virtual void BeginRecord(const String& file) { m_recording = true; }
		virtual void EndRecord() { m_recording = false; }
		bool IsRecording() const { return m_recording; }

	protected:
		int m_width;
		int m_height;
		int m_fps;
		bool m_recording;
	};
}