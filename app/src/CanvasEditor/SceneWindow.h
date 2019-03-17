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
#include "ui/Sprite.h"
#include "ui/Label.h"

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
                            canvas->SetName("Canvas");
                            camera->AddRenderer(canvas);
                        }

                        ImGui::EndMenu();
                    }

                    ImGui::EndMenu();
                }

                ImGui::EndPopup();
            }

            Vector<Ref<Renderer>> renderers = camera->GetRenderers();

            static int selection_mask = 0;
            int node_clicked = -1;
            ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize() * 3);
            for (int i = 0; i < renderers.Size(); ++i)
            {
                Ref<CanvasRenderer> canvas = RefCast<CanvasRenderer>(renderers[i]);
                const Vector<Ref<View>>& views = canvas->GetViews();

                ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ((selection_mask & (1 << i)) ? ImGuiTreeNodeFlags_Selected : 0);
                bool leaf = (views.Size() == 0);

                if (leaf)
                {
                    node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                }

                bool node_open = ImGui::TreeNodeEx((void*) (intptr_t) i, node_flags, canvas->GetName().CString());
                if (ImGui::IsItemClicked())
                {
                    node_clicked = i;
                }
                String id = String::Format("canvas context menu id %d", i);
                if (ImGui::BeginPopupContextItem(id.CString()))
                {
                    if (ImGui::BeginMenu("Create"))
                    {
                        if (ImGui::BeginMenu("UI"))
                        {
                            if (ImGui::MenuItem("View"))
                            {
                                auto view = RefMake<View>();
                                view->SetName("View");
                                canvas->AddView(view);
                            }

                            if (ImGui::MenuItem("Sprite"))
                            {
                                auto view = RefMake<Sprite>();
                                view->SetName("Sprite");
                                canvas->AddView(view);
                            }

                            if (ImGui::MenuItem("Label"))
                            {
                                auto view = RefMake<Label>();
                                view->SetName("Label");
                                canvas->AddView(view);
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
                        for (int j = 0; j < views.Size(); ++j)
                        {
                            DrawViewNode(views[j]);
                        }
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

        static void DrawViewNode(const Ref<View>& view)
        {
            ImGui::Text(view->GetName().CString());
        }
    };
}
