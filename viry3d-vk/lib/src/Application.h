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
#include <functional>

#define APP_VERSION_NAME "1.0.0"

namespace Viry3D
{
    typedef std::function<void()> Event;

    class ThreadPool;
    class ApplicationPrivate;

    class Application
    {
    public:
        static Application* Instance();
        Application();
        virtual ~Application();
        virtual void Update() { }
		virtual void OnResize(int width, int height) { }
        const String& GetName() const;
        void SetName(const String& name);
        const String& GetDataPath();
        const String& GetSavePath();
        ThreadPool* GetThreadPool() const;
        void PostEvent(Event event);
        void ProcessEvents();
        void OnFrameBegin();
        void OnFrameEnd();
        void Quit();
        bool HasQuit();

    private:
        ApplicationPrivate* m_private;
    };
}
