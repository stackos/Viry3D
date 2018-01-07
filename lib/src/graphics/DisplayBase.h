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
