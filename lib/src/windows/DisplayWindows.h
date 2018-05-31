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

#include "graphics/DisplayBase.h"
#include <Windows.h>

#if VR_GLES
#include "gles/gles_include.h"
#endif

namespace Viry3D
{
	class DisplayWindows: public DisplayBase
	{
	public:
		DisplayWindows();
		virtual ~DisplayWindows();
		void Init(int width, int height, int fps);
		void Deinit() { }
		void ProcessSystemEvents();
		void KeepScreenOn(bool enable) { }

	private:
		void CreateSystemWindow();

	protected:
		HWND m_window;
#if VR_GLES
		HDC m_hdc;
		HGLRC m_context;
		HGLRC m_shared_context;
#endif
	};
}
