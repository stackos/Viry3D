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

#include "Application.h"
#include "graphics/Display.h"
#include "graphics/Camera.h"
#include "ui/ImGuiRenderer.h"
#include "imgui/imgui.h"
#include "SceneWindow.h"

namespace Viry3D
{
    class CanvasEditor
    {
    public:
        Camera* m_ui_camera = nullptr;
        bool m_show_demo_window = false;
        Rect m_menu_rect;
        Rect m_scene_window_rect = Rect(0, 0, (float) (Display::Instance()->GetWidth() * 200 / 1280), (float) (Display::Instance()->GetHeight() * 450 / 720));
        Rect m_property_window_rect = Rect(0, 0, (float) (Display::Instance()->GetWidth() * 200 / 1280), 0);
        Rect m_view_window_rect = Rect(0, 0, 0, 0);
        Rect m_assets_window_rect = Rect(0, 0, (float) (Display::Instance()->GetWidth() * 640 / 1280), 0);
        Rect m_console_window_rect = Rect(0, 0, 0, 0);

        void InitUI()
        {
            m_ui_camera = Display::Instance()->CreateCamera();

            auto imgui = RefMake<ImGuiRenderer>();
            imgui->SetDrawAction([this]() {
                this->OnGUI();
            });
            m_ui_camera->AddRenderer(imgui);
        }

        void Init()
        {
            this->InitUI();
        }

        void Done()
        {
            if (m_ui_camera)
            {
                Display::Instance()->DestroyCamera(m_ui_camera);
                m_ui_camera = nullptr;
            }
        }

        void Update()
        {

        }

        void OnGUI()
        {
            this->ShowMainMenuBar();
            this->ShowSceneWindow();
            this->ShowPropertyWindow();
            this->ShowViewWindow();
            this->ShowAssetsWindow();
            this->ShowConsoleWindow();

            if (m_show_demo_window)
            {
                ImGui::ShowDemoWindow();
            }
        }

        void ShowMainMenuBar()
        {
            if (ImGui::BeginMainMenuBar())
            {
                auto pos = ImGui::GetWindowPos();
                auto size = ImGui::GetWindowSize();
                m_menu_rect = Rect(pos.x, pos.y, size.x, size.y);

                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("New")) { }

                    if (ImGui::MenuItem("Open", "Ctrl+O")) { }

                    if (ImGui::MenuItem("Save", "Ctrl+S")) { }

                    if (ImGui::MenuItem("Save As..")) { }

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Help"))
                {
                    ImGui::MenuItem("Demo Window", nullptr, &m_show_demo_window);
                    
                    ImGui::EndMenu();
                }

                ImGui::EndMainMenuBar();
            }
        }

        bool BeginMainWindow(const char* name, float x, float y, float w, float h, bool resize = true)
        {
            ImGui::SetNextWindowPos(ImVec2(x, y));
            ImGui::SetNextWindowSize(ImVec2(w, h));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleColor(ImGuiCol_TitleBg, (ImVec4) ImColor(20, 20, 20));
            ImGui::PushStyleColor(ImGuiCol_TitleBgActive, (ImVec4) ImColor(20, 20, 20));
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;
            if (!resize)
            {
                window_flags |= ImGuiWindowFlags_NoResize;
            }
            bool is_open = ImGui::Begin(name, nullptr, window_flags);
            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar(1);
            return is_open;
        }

        void EndMainWindow()
        {
            ImGui::End();
        }

        void ShowSceneWindow()
        {
            if (this->BeginMainWindow("Scene",
                m_menu_rect.x,
                m_menu_rect.h,
                m_scene_window_rect.w,
                m_scene_window_rect.h))
            {
                SceneWindow::OnGUI();
                
                auto pos = ImGui::GetWindowPos();
                auto size = ImGui::GetWindowSize();
                m_scene_window_rect = Rect(pos.x, pos.y, size.x, size.y);
                m_property_window_rect.h = m_scene_window_rect.h;
            }
            this->EndMainWindow();
        }

        void ShowPropertyWindow()
        {
            if (this->BeginMainWindow("Property",
                m_scene_window_rect.x + m_scene_window_rect.w,
                m_scene_window_rect.y,
                m_property_window_rect.w,
                m_property_window_rect.h))
            {
                auto pos = ImGui::GetWindowPos();
                auto size = ImGui::GetWindowSize();
                m_property_window_rect = Rect(pos.x, pos.y, size.x, size.y);
                m_view_window_rect.h = m_property_window_rect.h;
                m_view_window_rect.w = Display::Instance()->GetWidth() - m_scene_window_rect.w - m_property_window_rect.w;
            }
            this->EndMainWindow();
        }

        void ShowViewWindow()
        {
            if (this->BeginMainWindow("View",
                m_property_window_rect.x + m_property_window_rect.w,
                m_property_window_rect.y,
                m_view_window_rect.w,
                m_view_window_rect.h,
                false))
            {
                auto pos = ImGui::GetWindowPos();
                auto size = ImGui::GetWindowSize();
                m_view_window_rect = Rect(pos.x, pos.y, size.x, size.y);
                m_scene_window_rect.h = m_view_window_rect.h;
                m_assets_window_rect.y = m_view_window_rect.y + m_view_window_rect.h;
                m_assets_window_rect.h = Display::Instance()->GetHeight() - m_view_window_rect.y - m_view_window_rect.h;
            }
            this->EndMainWindow();
        }

        void ShowAssetsWindow()
        {
            ImGui::SetNextWindowSizeConstraints(ImVec2(0, -1), ImVec2(FLT_MAX, -1));
            if (this->BeginMainWindow("Assets",
                m_assets_window_rect.x,
                m_assets_window_rect.y,
                m_assets_window_rect.w,
                m_assets_window_rect.h))
            {
                auto pos = ImGui::GetWindowPos();
                auto size = ImGui::GetWindowSize();
                m_assets_window_rect = Rect(pos.x, pos.y, size.x, size.y);
                m_console_window_rect.x = m_assets_window_rect.w;
                m_console_window_rect.y = m_assets_window_rect.y;
                m_console_window_rect.w = Display::Instance()->GetWidth() - m_assets_window_rect.w;
                m_console_window_rect.h = m_assets_window_rect.h;
            }
            this->EndMainWindow();
        }

        void ShowConsoleWindow()
        {
            if (this->BeginMainWindow("Console",
                m_console_window_rect.x,
                m_console_window_rect.y,
                m_console_window_rect.w,
                m_console_window_rect.h,
                false))
            {
                auto pos = ImGui::GetWindowPos();
                auto size = ImGui::GetWindowSize();
                m_console_window_rect = Rect(pos.x, pos.y, size.x, size.y);
            }
            this->EndMainWindow();
        }
    };
}
