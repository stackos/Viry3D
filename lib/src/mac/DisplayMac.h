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
#include "thread/Thread.h"

namespace Viry3D {
    
class DisplayMac : public DisplayBase {
public:
    void Init(int width, int height, int fps);
    void Deinit();
    void* GetWindowBridge();
    void BindDefaultFramebuffer();
    void KeepScreenOn(bool enable) { }
    void OnWillResize(int width, int height);
    void DisplayLock() { m_mutex.lock(); }
    void DisplayUnlock() { m_mutex.unlock(); }
    int GetTargetWidth() const { return m_target_width; }
    int GetTargetHeight() const { return m_target_height; }
    void StopRender();
    
    void CreateSharedContext();
    void DestroySharedContext();
    
private:
    void* m_window;
    Mutex m_mutex;
    int m_target_width;
    int m_target_height;
};

}
