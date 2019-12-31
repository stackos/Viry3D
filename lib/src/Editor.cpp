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

#include "Engine.h"
#include "Editor.h"
#include "Debug.h"
#include "Input.h"
#include "GameObject.h"
#include "graphics/Camera.h"
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
            auto imgui = m_imgui.lock();
            if (!imgui)
            {
                auto imgui_camera = GameObject::Create("")->AddComponent<Camera>();
                imgui_camera->SetClearFlags(CameraClearFlags::Nothing);
                imgui_camera->SetDepth(0x7fffffff);
                imgui_camera->SetCullingMask(1 << 31);

                imgui = GameObject::Create("")->AddComponent<ImGuiRenderer>();
                imgui->GetGameObject()->SetLayer(31);
                imgui->SetDrawAction([this]() {
                    this->DrawWindows();
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

        if (m_editor_mode)
        {
            auto main_camera = Camera::GetMainCamera();
            if (main_camera)
            {
                if (Input::GetMouseButtonDown(0))
                {
                    auto pos = Input::GetMousePosition();
                    bool pos_in_window = false;

                    for (const auto& i : m_imgui_window_rects)
                    {
                        if (pos.x >= i.x &&
                            pos.x <= i.x + i.w &&
                            Engine::Instance()->GetHeight() - pos.y - 1 >= i.y &&
                            Engine::Instance()->GetHeight() - pos.y - 1 <= i.y + i.h)
                        {
                            pos_in_window = true;
                            break;
                        }
                    }

                    if (!pos_in_window)
                    {
                        Ray ray = main_camera->ScreenPointToRay(pos);
                        auto renderers = Renderer::GetRenderers();
                        Renderer* closest = nullptr;
                        float closest_length = Mathf::MaxFloatValue;

                        for (auto i : renderers)
                        {
                            if (i->GetGameObject()->IsActiveInTree() && i->IsEnable())
                            {
                                auto local_bounds = i->GetLocalBounds();
                                if (local_bounds.GetSize().SqrMagnitude() > 0)
                                {
                                    const auto& world_to_local = i->GetTransform()->GetWorldToLocalMatrix();
                                    Ray ray_in_local(world_to_local.MultiplyPoint3x4(ray.GetOrigin()), world_to_local.MultiplyDirection(ray.GetDirection()));
                                    float ray_length = main_camera->GetFarClip();
                                    if (Mathf::RayBoundsIntersection(ray_in_local, local_bounds, ray_length))
                                    {
                                        auto hit = ray.GetPoint(ray_length);
                                        if (ray_length < closest_length)
                                        {
                                            closest = i;
                                            closest_length = ray_length;
                                        }
                                    }
                                }
                            }
                        }

                        if (closest)
                        {
                            auto obj = closest->GetGameObject();
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
    }

    void Editor::DrawWindows()
    {
        ImGui::ShowDemoWindow();

        m_imgui_window_rects.Clear();

        auto selected_object = m_selected_object.lock();
        if (selected_object)
        {
            float inspector_window_w = 300;
            ImGui::SetNextWindowPos(ImVec2((float) Engine::Instance()->GetWidth() - inspector_window_w, 0), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(inspector_window_w, (float) Engine::Instance()->GetHeight()), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Inspector", nullptr, 0))
            {
                if (ImGui::CollapsingHeader("GameObject", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    static const int NAME_SIZE_MAX = 1024;
                    static char name[NAME_SIZE_MAX] = "";
                    assert(selected_object->GetName().Size() < NAME_SIZE_MAX);
                    strcpy(name, selected_object->GetName().CString());
                    if (ImGui::InputText("Name", name, NAME_SIZE_MAX))
                    {
                        selected_object->SetName(name);
                    }
                }

                if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    Vector3 pos = selected_object->GetTransform()->GetLocalPosition();
                    if (ImGui::DragFloat3("Position", (float*) &pos, 0.1f))
                    {
                        selected_object->GetTransform()->SetLocalPosition(pos);
                    }

                    static Vector3 rot_set;
                    Vector3 rot;
                    if (Mathf::FloatEqual(Mathf::Abs(selected_object->GetTransform()->GetLocalRotation().Dot(Quaternion::Euler(rot_set))), 1))
                    {
                        rot = rot_set;
                    }
                    else
                    {
                        rot = selected_object->GetTransform()->GetLocalRotation().ToEulerAngles();
                    }
                    if (ImGui::DragFloat3("Rotation", (float*) &rot, 1.0f))
                    {
                        selected_object->GetTransform()->SetLocalRotation(Quaternion::Euler(rot));
                        rot_set = rot;
                    }

                    Vector3 scale = selected_object->GetTransform()->GetLocalScale();
                    if (ImGui::DragFloat3("Scale", (float*) &scale, 0.1f))
                    {
                        selected_object->GetTransform()->SetLocalScale(scale);
                    }
                }

                auto pos = ImGui::GetWindowPos();
                auto size = ImGui::GetWindowSize();
                m_imgui_window_rects.Add(Rect(pos.x, pos.y, size.x, size.y));
            }
            ImGui::End();
        }
    }
}
