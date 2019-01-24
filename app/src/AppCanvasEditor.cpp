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

#include "App.h"
#include "CanvasEditor/CanvasEditor.h"

namespace Viry3D
{
    class AppImplement
    {
    public:
        CanvasEditor* m_editor = nullptr;

        void Init()
        {
            m_editor = new CanvasEditor();
            m_editor->Init();
        }

        ~AppImplement()
        {
            m_editor->Done();
            delete m_editor;
        }

        void Update()
        {
            m_editor->Update();
        }
    };

    App::App()
    {
        m_app = new AppImplement();
    }

    App::~App()
    {
        delete m_app;
    }

    void App::Init()
    {
        m_app->Init();
    }

    void App::Update()
    {
        m_app->Update();
    }
}
