/*
* Viry3D
* Copyright 2014-2017 by Stack - stackos@qq.com
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

#include "Application.h"

#if VR_WINDOWS
#include <Windows.h>
#define VR_MAIN(app_class) \
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) \
{ \
	Ref<app_class> app = RefMake<app_class>(); \
	app->Run(); \
    return 0; \
}
#endif

#if VR_IOS
#import <UIKit/UIKit.h>
#define VR_MAIN(app_class) \
int main(int argc, char * argv[]) \
{ \
    @autoreleasepool { \
        Ref<app_class> app = RefMake<app_class>(); \
        int result = UIApplicationMain(argc, argv, nil, @"AppDelegate"); \
        return result; \
    } \
}
#endif

#if VR_MAC
#import <Cocoa/Cocoa.h>
@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
@end
#define VR_MAIN(app_class) \
int main(int argc, char * argv[]) \
{ \
    Ref<app_class> app = RefMake<app_class>(); \
    NSApplication* _app = [NSApplication sharedApplication]; \
    AppDelegate* appDelegate = [[AppDelegate alloc] init]; \
    _app.delegate = appDelegate; \
    [_app run]; \
    return 0; \
} \
}
#endif

#if VR_ANDROID
#define VR_MAIN(app_class) \
Ref<Application> viry3d_android_main() \
{ \
	return RefMake<app_class>(); \
}
#endif
