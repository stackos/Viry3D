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
        m_imgui_camera = Display::Instance()->CreateCamera();
        m_imgui_camera->SetDepth(0x7fffffff);

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
        for (int i = 0; i < m_cameras.Size(); ++i)
        {
            Display::Instance()->DestroyCamera(m_cameras[i]);
        }
        m_cameras.Clear();

        if (m_imgui_camera)
        {
            Display::Instance()->DestroyCamera(m_imgui_camera);
            m_imgui_camera.reset();
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
                if (ImGui::MenuItem("Quit", "Alt+F4"))
                {
                    Application::Instance()->Quit();
                }

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
            if (!m_view_rt || m_view_rt->GetWidth() != view_w || m_view_rt->GetHeight() != view_h)
            {
                m_view_rt = Texture::CreateRenderTexture(
                    view_w,
                    view_h,
                    TextureFormat::R8G8B8A8,
                    1,
                    1,
                    true,
                    FilterMode::Nearest,
                    SamplerAddressMode::ClampToEdge);
                m_view_depth = Texture::CreateRenderTexture(
                    view_w,
                    view_h,
                    Texture::ChooseDepthFormatSupported(true),
                    1,
                    1,
                    true,
                    FilterMode::Nearest,
                    SamplerAddressMode::ClampToEdge);
                for (int i = 0; i < m_cameras.Size(); ++i)
                {
                    m_cameras[i]->SetRenderTarget(m_view_rt, m_view_depth);
                    m_cameras[i]->OnResize(view_w, view_h);
                }
            }

            ImGui::Image(&m_view_rt, view_size);
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

    Ref<Camera> CanvasEditor::CreateCamera()
    {
        Ref<Camera> camera = Display::Instance()->CreateCamera();
        if (m_view_rt)
        {
            camera->SetRenderTarget(m_view_rt, m_view_depth);
        }
        m_cameras.Add(camera);
        return camera;
    }

    void CanvasEditor::DestroyCamera(const Ref<Camera>& camera)
    {
        Display::Instance()->DestroyCamera(camera);
        m_cameras.Remove(camera);
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

    Ref<Object> CanvasEditor::FindView(const Ref<View>& view, uint32_t id)
    {
        Ref<Object> find;

        if (view->GetId() == id)
        {
            find = view;
        }
        else
        {
            for (int i = 0; i < view->GetSubviewCount(); ++i)
            {
                find = this->FindView(view->GetSubview(i), id);
                if (find)
                {
                    break;
                }
            }
        }

        return find;
    }

    Ref<Object> CanvasEditor::FindNode(const Ref<Node>& node, uint32_t id)
    {
        Ref<Object> find;

        if (node->GetId() == id)
        {
            find = node;
        }
        else
        {
            Ref<CanvasRenderer> canvas = RefCast<CanvasRenderer>(node);
            if (canvas)
            {
                const Vector<Ref<View>>& views = canvas->GetViews();
                for (int i = 0; i < views.Size(); ++i)
                {
                    find = this->FindView(views[i], id);
                    if (find)
                    {
                        break;
                    }
                }
            }
            else
            {
                for (int i = 0; i < node->GetChildCount(); ++i)
                {
                    find = this->FindNode(node->GetChild(i), id);
                    if (find)
                    {
                        break;
                    }
                }
            }
        }

        return find;
    }

    Ref<Object> CanvasEditor::GetSelectionObject(uint32_t id)
    {
        Ref<Object> obj;

        for (int i = 0; i < m_cameras.Size(); ++i)
        {
            if (m_cameras[i]->GetId() == id)
            {
                obj = m_cameras[i];
                break;
            }

            const Vector<Ref<Node>>& nodes = m_cameras[i]->GetNodes();
            for (int j = 0; j < nodes.Size(); ++j)
            {
                obj = this->FindNode(nodes[j], id);
                if (obj)
                {
                    break;
                }
            }
        }

        return obj;
    }

	String CanvasEditor::OpenFilePanel(const String& initial_path, const char* filter)
	{
		String file_path;

#if VR_WINDOWS
		char path[MAX_PATH];
		String data_path = initial_path.Replace("/", "\\");
		strcpy(path, data_path.CString());

		OPENFILENAME open = { };
		open.lStructSize = sizeof(open);
		open.hwndOwner = (HWND) Display::Instance()->GetWindow();
		open.lpstrFilter = filter;
		open.lpstrFile = path;
		open.nMaxFile = MAX_PATH;
		open.nFilterIndex = 0;
		open.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

		if (GetOpenFileName(&open))
		{
			file_path = path;
			file_path = file_path.Replace("\\", "/");
		}
#endif

		return file_path;
	}
}
