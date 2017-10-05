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

#include "DisplayMac.h"

namespace Viry3D
{
    
void DisplayMac::Init(int width, int height, int fps)
{
    CGRect bounds = [UIScreen mainScreen].bounds;
    float scale = [UIScreen mainScreen].nativeScale;
    
    DisplayBase::Init(bounds.size.width * scale, bounds.size.height * scale, fps);
    
    g_view_controller = [[ViewController alloc] init];
    if(fps <= 0) {
        fps = 60;
    }
    g_view_controller.preferredFramesPerSecond = fps;
    
    UIWindow* window = [[UIWindow alloc] initWithFrame:bounds];
    window.rootViewController = g_view_controller;
    [window makeKeyAndVisible];
    
    m_window = (void*) CFBridgingRetain(window);
}
    
}
