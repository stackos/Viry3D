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

#if VR_WINDOWS
#include <Windows.h>
#elif VR_IOS
#import <UIKit/UIKit.h>
#elif VR_MAC
#import <Cocoa/Cocoa.h>
#elif VR_ANDROID
#include <android/log.h>
#endif

namespace Viry3D
{
#if VR_WINDOWS
    void Debug::LogString(const String& str, bool end_line)
    {
        if (end_line)
        {
            OutputDebugString((str + "\n").CString());
        }
        else
        {
            OutputDebugString(str.CString());
        }
    }
#elif VR_IOS
    void Debug::LogString(const String& str, bool end_line)
    {
        NSLog(@"\n%s", str.CString());
    }
#elif VR_MAC
    void Debug::LogString(const String& str, bool end_line)
    {
        NSLog(@"\n%s", str.CString());
    }
#elif VR_ANDROID
    void Debug::LogString(const String& str, bool end_line)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Viry3D", "%s", str.CString());
    }
#endif
}
