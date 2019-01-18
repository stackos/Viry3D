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

#pragma once

#include "Demo.h"
#include "Application.h"
#include "graphics/Display.h"
#include "graphics/Camera.h"
#include "ui/ImGuiRenderer.h"
#include "imgui/imgui.h"

namespace Viry3D
{
    class DemoCanvasEditor : public Demo
    {
    public:
        Camera* m_ui_camera = nullptr;

        void InitUI()
        {
            m_ui_camera = Display::Instance()->CreateCamera();

#if VR_WINDOWS
            auto imgui = RefMake<ImGuiRenderer>();
            imgui->SetDrawAction([this]() {
                this->OnGUI();
            });
            m_ui_camera->AddRenderer(imgui);
#endif
        }

        virtual void Init()
        {
            this->InitUI();
        }

        virtual void Done()
        {
            if (m_ui_camera)
            {
                Display::Instance()->DestroyCamera(m_ui_camera);
                m_ui_camera = nullptr;
            }
        }

        virtual void Update()
        {

        }

        void OnGUI()
        {
            ImGui::ShowDemoWindow();
        }
    };
}
