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

#include "Editor.h"
#include "Debug.h"
#include "Input.h"
#include "GameObject.h"
#include "graphics/Camera.h"
#include "physics/Physics.h"
#include "physics/Collider.h"
#include "ui/ImGuiRenderer.h"
#include "imgui/imgui.h"

namespace Viry3D
{
    void Editor::Update()
    {
        if (Input::GetKeyDown(KeyCode::E) && (Input::GetKey(KeyCode::LeftControl) || Input::GetKey(KeyCode::RightControl)))
        {
            m_editor_mode = !m_editor_mode;

            if (m_editor_mode)
            {
                Log("enter editor mode");
            }
            else
            {
                Log("exit editor mode");
            }
        }

        if (m_editor_mode)
        {
            auto main_camera = Camera::GetMainCamera();
            if (main_camera)
            {
                if (Input::GetMouseButtonDown(0))
                {
                    auto pos = Input::GetMousePosition();
                    Ray ray = main_camera->ScreenPointToRay(pos);
                    RaycastHit hit;
                    if (Physics::Raycast(hit, ray.GetOrigin(), ray.GetDirection(), main_camera->GetFarClip()))
                    {
                        auto col = hit.collider.lock();
                        if (col)
                        {
                            auto obj = col->GetGameObject();
                            if (obj == m_selected_object.lock())
                            {
                                auto root = obj->GetTransform()->GetRoot()->GetGameObject();
                                Log("Select:%s", root->GetName().CString());
                                m_selected_object = root;
                            }
                            else
                            {
                                Log("Select:%s", obj->GetName().CString());
                                m_selected_object = obj;
                            }
                        }
                    }
                }
            }
        }

        if (m_editor_mode)
        {
            auto imgui = m_imgui.lock();
            if (!imgui)
            {
                auto imgui_camera = GameObject::Create("")->AddComponent<Camera>();
                imgui_camera->SetClearFlags(CameraClearFlags::Nothing);
                imgui_camera->SetDepth(0x7fffffff);
                imgui_camera->SetCullingMask(1 << 31);

                imgui = GameObject::Create("")->AddComponent<ImGuiRenderer>();
                imgui->GetGameObject()->SetLayer(31);
                imgui->SetDrawAction([]() {
                    ImGui::ShowDemoWindow();
                });
                imgui->SetCamera(imgui_camera);
                m_imgui = imgui;
            }

            if (!imgui->IsEnable())
            {
                imgui->Enable(true);
            }

            imgui->UpdateImGui();
        }
        else
        {
            auto imgui = m_imgui.lock();
            if (imgui)
            {
                if (imgui->IsEnable())
                {
                    imgui->Enable(false);
                }
            }
        }
    }
}
