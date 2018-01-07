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

#include "Debug.h"

#if VR_ANDROID
#include <android/log.h>
#elif VR_WINDOWS
#include <Windows.h>
#endif

namespace Viry3D
{
#if VR_ANDROID || VR_WINDOWS
	void Debug::LogString(const String& str, bool end_line)
	{
#if VR_ANDROID
		__android_log_print(ANDROID_LOG_ERROR, "Viry3D", "%s", str.CString());
#elif VR_WINDOWS
		if (end_line)
		{
			OutputDebugString((str + "\n").CString());
		}
		else
		{
			OutputDebugString(str.CString());
		}
#endif
	}
#endif
}
