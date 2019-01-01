/*
* Viry3D
* Copyright 2014-2019 by Stack - stackos@qq.com
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
#include "graphics/Display.h"
#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "ui/Font.h"
#include "audio/AudioManager.h"
#include "Debug.h"

#if VR_WINDOWS
#include <Windows.h>
#elif VR_IOS
#import <UIKit/UIKit.h>
#elif VR_MAC
#import <Cocoa/Cocoa.h>
#elif VR_ANDROID
#include "android/jni.h"
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
        List<Action> m_actions;
        Mutex m_mutex;
        bool m_quit;
        Ref<ThreadPool> m_thread_pool;
#if VR_GLES
        Ref<ThreadPool> m_resource_thread_pool;
#endif

        ApplicationPrivate(Application* app):
            m_quit(false)
        {
            m_app = app;
#if !VR_WASM
            m_thread_pool = RefMake<ThreadPool>(8);
#if VR_GLES
            m_resource_thread_pool = RefMake<ThreadPool>(1,
                []() {
                    Display::Instance()->BindSharedContext();
                },
                []() {
                    Display::Instance()->UnbindSharedContext();
                });
#endif
#endif
            Font::Init();
            AudioManager::Init();
        }

        ~ApplicationPrivate()
        {
            AudioManager::Done();
            Font::Done();
			Texture::Done();
			Shader::Done();
            m_thread_pool.reset();
#if VR_GLES
            m_resource_thread_pool.reset();
#endif
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
#elif VR_IOS
    const String& Application::GetDataPath()
    {
        if (m_private->m_data_path.Empty())
        {
            String path = [[[NSBundle mainBundle] bundlePath] UTF8String];
            path += "/Assets";
            m_private->m_data_path = path;
        }

        return m_private->m_data_path;
    }
    
    const String& Application::GetSavePath()
    {
        if (m_private->m_save_path.Empty())
        {
            NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
            NSString* doc_path = paths[0];
            m_private->m_save_path = [doc_path UTF8String];
        }
        
        return m_private->m_save_path;
    }
#elif VR_MAC
    const String& Application::GetDataPath()
    {
        if (m_private->m_data_path.Empty())
        {
            String path = [[[NSBundle mainBundle] resourcePath] UTF8String];
            path += "/Assets";
            m_private->m_data_path = path;
        }
        
        return m_private->m_data_path;
    }
    
    const String& Application::GetSavePath()
    {
        if (m_private->m_save_path.Empty())
        {
            NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
            NSString* doc_path = [paths objectAtIndex:0];
            m_private->m_save_path = [doc_path UTF8String];
        }
        
        return m_private->m_save_path;
    }
#elif VR_ANDROID
    const String& Application::GetDataPath()
    {
        return m_private->m_data_path;
    }

    const String& Application::GetSavePath()
    {
        return m_private->m_save_path;
    }

    void Application::SetDataPath(const String& path)
    {
        m_private->m_data_path = path;
    }

    void Application::SetSavePath(const String& path)
    {
        m_private->m_save_path = path;
    }
#elif VR_WASM
    const String& Application::GetDataPath()
    {
        if (m_private->m_data_path.Empty())
        {
            m_private->m_data_path = "Assets";
        }
        return m_private->m_data_path;
    }

    const String& Application::GetSavePath()
    {
        Log("web has no save path");

        return m_private->m_save_path;
    }
#elif VR_UWP
const String& Application::GetDataPath()
{
    return m_private->m_data_path;
}

const String& Application::GetSavePath()
{
    return m_private->m_save_path;
}

void Application::SetDataPath(const String& path)
{
    m_private->m_data_path = path;
}

void Application::SetSavePath(const String& path)
{
    m_private->m_save_path = path;
}
#endif

    ThreadPool* Application::GetThreadPool() const
    {
        return m_private->m_thread_pool.get();
    }

#if VR_GLES
    ThreadPool* Application::GetResourceThreadPool() const
    {
        return m_private->m_resource_thread_pool.get();
    }
#endif

    void Application::PostAction(Action action)
    {
        m_private->m_mutex.lock();
        m_private->m_actions.AddLast(action);
        m_private->m_mutex.unlock();
    }

    void Application::ProcessActions()
    {
        m_private->m_mutex.lock();
        for (const auto& action : m_private->m_actions)
        {
            if (action)
            {
                action();
            }
        }
        m_private->m_actions.Clear();
        m_private->m_mutex.unlock();
    }

    void Application::OnFrameBegin()
    {
        Time::Update();
        this->ProcessActions();
    }

    void Application::OnFrameEnd()
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
