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
#include "Input.h"
#include "container/List.h"
#include "thread/ThreadPool.h"
#include "time/Time.h"
#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "ui/Font.h"

#if VR_WINDOWS
#include <Windows.h>
#endif

namespace Viry3D
{
    class ApplicationPrivate
    {
    public:
        static Application* m_app;
        String m_name;
        String m_data_path;
        String m_save_path;
        List<Event> m_events;
        Mutex m_mutex;
        Ref<ThreadPool> m_thread_pool;
        bool m_quit;

        ApplicationPrivate(Application* app):
            m_quit(false)
        {
            m_app = app;
            m_thread_pool = RefMake<ThreadPool>(8);
            Font::Init();
        }

        ~ApplicationPrivate()
        {
            Font::Done();
			Texture::Done();
			Shader::Done();
            m_thread_pool.reset();
            m_app = nullptr;
        }
    };

    Application* ApplicationPrivate::m_app;

    Application* Application::Instance()
    {
        return ApplicationPrivate::m_app;
    }

    Application::Application():
        m_private(new ApplicationPrivate(this))
    {
        this->GetDataPath();
        this->GetSavePath();
    }

    Application::~Application()
    {
        delete m_private;
    }

    const String& Application::GetName() const
    {
        return m_private->m_name;
    }

    void Application::SetName(const String& name)
    {
        m_private->m_name = name;
    }

#if VR_WINDOWS
    const String& Application::GetDataPath()
    {
        if (m_private->m_data_path.Empty())
        {
            char buffer[MAX_PATH];
            ::GetModuleFileName(nullptr, buffer, MAX_PATH);
            String path = buffer;
            path = path.Replace("\\", "/").Substring(0, path.LastIndexOf("\\")) + "/Assets";
            m_private->m_data_path = path;
        }

        return m_private->m_data_path;
    }

    const String& Application::GetSavePath()
    {
        if (m_private->m_save_path.Empty())
        {
            m_private->m_save_path = this->GetDataPath();
        }

        return m_private->m_save_path;
    }
#endif

    ThreadPool* Application::GetThreadPool() const
    {
        return m_private->m_thread_pool.get();
    }

    void Application::PostEvent(Event event)
    {
        m_private->m_mutex.lock();
        m_private->m_events.AddLast(event);
        m_private->m_mutex.unlock();
    }

    void Application::ProcessEvents()
    {
        m_private->m_mutex.lock();
        for (const auto& event : m_private->m_events)
        {
            if (event)
            {
                event();
            }
        }
        m_private->m_events.Clear();
        m_private->m_mutex.unlock();
    }

    void Application::UpdateBegin()
    {
        Time::Update();
        this->ProcessEvents();
    }

    void Application::UpdateEnd()
    {
#if VR_ANDROID
        if (Input::GetKeyDown(KeyCode::Backspace))
#else
        if (Input::GetKeyDown(KeyCode::Escape))
#endif
        {
            this->Quit();
        }

        Input::Update();
    }

    void Application::Quit()
    {
        m_private->m_quit = true;

#if VR_ANDROID
        java_quit_application();
#endif
    }

    bool Application::HasQuit()
    {
        return m_private->m_quit;
    }
}
