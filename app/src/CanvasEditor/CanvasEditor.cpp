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

#include "CanvasEditor.h"
#include "imgui/imgui.h"
#include "Application.h"
#include "graphics/Display.h"
#include "graphics/Camera.h"
#include "graphics/Texture.h"
#include "ui/ImGuiRenderer.h"
#include "memory/Memory.h"
#include "SceneWindow.h"
#include "PropertyWindow.h"

namespace Viry3D
{
    void CanvasEditor::InitUI()
    {
        m_ui_camera = Display::Instance()->CreateCamera();
        m_ui_camera->SetDepth(0);

        m_imgui_camera = Display::Instance()->CreateCamera();
        m_imgui_camera->SetDepth(1);

        m_imgui = RefMake<ImGuiRenderer>();
        m_imgui->SetDrawAction([this]() {
            this->OnGUI();
        });
        m_imgui_camera->AddRenderer(m_imgui);
    }

    void CanvasEditor::Init()
    {
        this->InitUI();
    }

    void CanvasEditor::Done()
    {
        if (m_ui_camera)
        {
            Display::Instance()->DestroyCamera(m_ui_camera);
            m_ui_camera = nullptr;
        }

        if (m_imgui_camera)
        {
            Display::Instance()->DestroyCamera(m_imgui_camera);
            m_imgui_camera = nullptr;
        }
    }

    void CanvasEditor::Update()
    {
        m_imgui->UpdateImGui();
    }

    void CanvasEditor::OnGUI()
    {
        this->ShowMainMenuBar();
        this->ShowSceneWindow();
        this->ShowPropertyWindow();
        this->ShowViewWindow();
        this->ShowConsoleWindow();

        if (m_show_demo_window)
        {
            ImGui::ShowDemoWindow();
        }
    }

    void CanvasEditor::ShowMainMenuBar()
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

    bool CanvasEditor::BeginMainWindow(const char* name, float x, float y, float w, float h, bool resize)
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

    void CanvasEditor::EndMainWindow()
    {
        ImGui::End();
    }

    void CanvasEditor::ShowSceneWindow()
    {
        if (this->BeginMainWindow("Scene",
            m_menu_rect.x,
            m_menu_rect.h,
            m_scene_window_rect.w,
            m_scene_window_rect.h))
        {
            SceneWindow::OnGUI(this);

            auto pos = ImGui::GetWindowPos();
            auto size = ImGui::GetWindowSize();
            m_scene_window_rect = Rect(pos.x, pos.y, size.x, size.y);
            m_property_window_rect.h = m_scene_window_rect.h;
        }
        this->EndMainWindow();
    }

    void CanvasEditor::ShowPropertyWindow()
    {
        if (this->BeginMainWindow("Property",
            m_scene_window_rect.x + m_scene_window_rect.w,
            m_scene_window_rect.y,
            m_property_window_rect.w,
            m_property_window_rect.h))
        {
            PropertyWindow::OnGUI(this);

            auto pos = ImGui::GetWindowPos();
            auto size = ImGui::GetWindowSize();
            m_property_window_rect = Rect(pos.x, pos.y, size.x, size.y);
            m_view_window_rect.h = m_property_window_rect.h;
            m_view_window_rect.w = Display::Instance()->GetWidth() - m_scene_window_rect.w - m_property_window_rect.w;
        }
        this->EndMainWindow();
    }

    void CanvasEditor::ShowViewWindow()
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
            m_console_window_rect.y = m_view_window_rect.y + m_view_window_rect.h;
            m_console_window_rect.w = (float) Display::Instance()->GetWidth();
            m_console_window_rect.h = Display::Instance()->GetHeight() - m_view_window_rect.y - m_view_window_rect.h;

            ImVec2 view_size = { size.x - ImGui::GetStyle().WindowPadding.x * 2, size.y - ImGui::GetCursorPos().y - ImGui::GetStyle().WindowPadding.y };
            int view_w = (int) view_size.x;
            int view_h = (int) view_size.y;
            if (!m_ui_rt || m_ui_rt->GetWidth() != view_w || m_ui_rt->GetHeight() != view_h)
            {
                m_ui_rt = Texture::CreateRenderTexture(
                    view_w,
                    view_h,
                    TextureFormat::R8G8B8A8,
                    1,
                    1,
                    true,
                    FilterMode::Nearest,
                    SamplerAddressMode::ClampToEdge);
                m_ui_camera->SetRenderTarget(m_ui_rt, Ref<Texture>());
                m_ui_camera->OnResize(view_w, view_h);
            }

            ImGui::Image(&m_ui_rt, view_size);
        }
        this->EndMainWindow();
    }

    void CanvasEditor::ShowConsoleWindow()
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

    ByteBuffer& CanvasEditor::GetTextBuffer(const String& name)
    {
        ByteBuffer* ptr = nullptr;
        if (!m_text_buffer.TryGet(name, &ptr))
        {
            ByteBuffer buffer(1024);
            Memory::Zero(buffer.Bytes(), buffer.Size());
            m_text_buffer.Add(name, buffer);
            m_text_buffer.TryGet(name, &ptr);
        }
        assert(ptr);
        return *ptr;
    }

    Ref<View> CanvasEditor::FindView(const Vector<Ref<View>>& views, uint32_t id)
    {
        Ref<View> view;

        for (int i = 0; i < views.Size(); ++i)
        {
            if (views[i]->GetId() == id)
            {
                view = views[i];
                break;
            }
            else
            {
                view = this->FindView(views[i]->GetSubviews(), id);
                if (view)
                {
                    break;
                }
            }
        }

        return view;
    }

    Ref<Object> CanvasEditor::GetSelectionObject(uint32_t id)
    {
        Ref<Object> obj;

        Vector<Ref<Renderer>> renderers = this->GetCamera()->GetRenderers();
        for (int i = 0; i < renderers.Size(); ++i)
        {
            Ref<CanvasRenderer> canvas = RefCast<CanvasRenderer>(renderers[i]);
            
            if (canvas->GetId() == id)
            {
                obj = canvas;
                break;
            }
            else
            {
                Ref<View> view = this->FindView(canvas->GetViews(), id);
                if (view)
                {
                    obj = view;
                    break;
                }
            }
        }

        return obj;
    }
}
