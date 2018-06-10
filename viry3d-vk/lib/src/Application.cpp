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
    };

    String ApplicationPrivate::m_name;
    String ApplicationPrivate::m_data_path;
    String ApplicationPrivate::m_save_path;

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
            ::GetModuleFileName(NULL, buffer, MAX_PATH);
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
}
