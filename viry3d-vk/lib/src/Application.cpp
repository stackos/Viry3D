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

#include "Application.h"
#include "container/List.h"
#include "thread/ThreadPool.h"

#if VR_WINDOWS
#include <Windows.h>
#endif

namespace Viry3D
{
    class ApplicationPrivate
    {
    public:
        static String m_name;
        static String m_data_path;
        static String m_save_path;
        static List<Event> m_events;
        static Mutex m_mutex;
    };

    String ApplicationPrivate::m_name;
    String ApplicationPrivate::m_data_path;
    String ApplicationPrivate::m_save_path;
    List<Event> ApplicationPrivate::m_events;
    Mutex ApplicationPrivate::m_mutex;

    void Application::SetName(const String& name)
    {
        ApplicationPrivate::m_name = name;
    }

    const String& Application::Name()
    {
        return ApplicationPrivate::m_name;
    }

#if VR_WINDOWS
    const String& Application::DataPath()
    {
        if (ApplicationPrivate::m_data_path.Empty())
        {
            char buffer[MAX_PATH];
            ::GetModuleFileName(nullptr, buffer, MAX_PATH);
            String path = buffer;
            path = path.Replace("\\", "/").Substring(0, path.LastIndexOf("\\")) + "/Assets";
            ApplicationPrivate::m_data_path = path;
        }

        return ApplicationPrivate::m_data_path;
    }

    const String& Application::SavePath()
    {
        if (ApplicationPrivate::m_save_path.Empty())
        {
            ApplicationPrivate::m_save_path = DataPath();
        }

        return ApplicationPrivate::m_save_path;
    }
#endif

    void Application::PostEvent(Event event)
    {
        ApplicationPrivate::m_mutex.lock();
        ApplicationPrivate::m_events.AddLast(event);
        ApplicationPrivate::m_mutex.unlock();
    }

    void Application::ProcessEvents()
    {
        ApplicationPrivate::m_mutex.lock();
        for (const auto& event : ApplicationPrivate::m_events)
        {
            if (event)
            {
                event();
            }
        }
        ApplicationPrivate::m_events.Clear();
        ApplicationPrivate::m_mutex.unlock();
    }
}
