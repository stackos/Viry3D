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

#include "imgui/imgui.h"
#include "graphics/Camera.h"
#include "ui/CanvasRenderer.h"

namespace Viry3D
{
    class SceneWindow
    {
    public:
        static void OnGUI(Camera* camera)
        {
            if (ImGui::BeginPopupContextWindow())
            {
                if (ImGui::BeginMenu("Create"))
                {
                    if (ImGui::BeginMenu("UI"))
                    {
                        if (ImGui::MenuItem("Canvas"))
                        {
                            auto canvas = RefMake<CanvasRenderer>(FilterMode::Nearest);
                            camera->AddRenderer(canvas);
                        }

                        ImGui::EndMenu();
                    }

                    ImGui::EndMenu();
                }

                ImGui::EndPopup();
            }

            static int selection_mask = 0;
            int node_clicked = -1;
            ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize() * 3);
            for (int i = 0; i < 5; ++i)
            {
                ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ((selection_mask & (1 << i)) ? ImGuiTreeNodeFlags_Selected : 0);
                bool leaf = false;

                if (leaf)
                {
                    node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                }

                bool node_open = ImGui::TreeNodeEx((void*) (intptr_t) i, node_flags, "Node");
                if (ImGui::IsItemClicked())
                {
                    node_clicked = i;
                }
                String id = String::Format("id %d", i);
                if (ImGui::BeginPopupContextItem(id.CString()))
                {
                    if (ImGui::BeginMenu("Create"))
                    {
                        if (ImGui::BeginMenu("UI"))
                        {
                            if (ImGui::MenuItem("View"))
                            {

                            }

                            if (ImGui::MenuItem("Sprite"))
                            {

                            }

                            if (ImGui::MenuItem("Label"))
                            {

                            }

                            ImGui::EndMenu();
                        }

                        ImGui::EndMenu();
                    }

                    ImGui::EndPopup();
                }

                if (!leaf)
                {
                    if (node_open)
                    {
                        ImGui::Text("Blah blah");
                        ImGui::TreePop();
                    }
                }
            }
            if (node_clicked != -1)
            {
                if (ImGui::GetIO().KeyCtrl)
                {
                    selection_mask ^= (1 << node_clicked);
                }
                else
                {
                    selection_mask = (1 << node_clicked);
                }
            }
            ImGui::PopStyleVar();
        }
    };
}
