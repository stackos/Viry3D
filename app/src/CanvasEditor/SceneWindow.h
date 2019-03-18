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

            static Vector<uint32_t> selections;
            int node_clicked = -1;
            for (int i = 0; i < renderers.Size(); ++i)
            {
                Ref<CanvasRenderer> canvas = RefCast<CanvasRenderer>(renderers[i]);
                const Vector<Ref<View>>& views = canvas->GetViews();

                ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (selections.Contains(canvas->GetId()) ? ImGuiTreeNodeFlags_Selected : 0);
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
                
                String id = String::Format("canvas context menu id %d", canvas->GetId());
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
                        DrawViewNodes(views, selections);
                        ImGui::TreePop();
                    }
                }
            }
            if (node_clicked != -1)
            {
                uint32_t node_id = renderers[node_clicked]->GetId();
                if (ImGui::GetIO().KeyCtrl)
                {
                    if (selections.Contains(node_id))
                    {
                        selections.Remove(node_id);
                    }
                    else
                    {
                        selections.Add(node_id);
                    }
                }
                else
                {
                    selections.Clear();
                    selections.Add(node_id);
                }
            }
        }

        static void DrawViewNodes(const Vector<Ref<View>>& views, Vector<uint32_t>& selections)
        {
            int node_clicked = -1;
            for (int i = 0; i < views.Size(); ++i)
            {
                Ref<View> view = views[i];
                const Vector<Ref<View>>& subviews = view->GetSubviews();

                ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (selections.Contains(view->GetId()) ? ImGuiTreeNodeFlags_Selected : 0);
                bool leaf = (subviews.Size() == 0);

                if (leaf)
                {
                    node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                }

                bool node_open = ImGui::TreeNodeEx((void*) (intptr_t) i, node_flags, view->GetName().CString());
                if (ImGui::IsItemClicked())
                {
                    node_clicked = i;
                }

                String id = String::Format("view context menu id %d", view->GetId());
                if (ImGui::BeginPopupContextItem(id.CString()))
                {
                    if (ImGui::BeginMenu("Create"))
                    {
                        if (ImGui::BeginMenu("UI"))
                        {
                            if (ImGui::MenuItem("View"))
                            {
                                auto subview = RefMake<View>();
                                subview->SetName("View");
                                view->AddSubview(subview);
                            }

                            if (ImGui::MenuItem("Sprite"))
                            {
                                auto subview = RefMake<Sprite>();
                                subview->SetName("Sprite");
                                view->AddSubview(subview);
                            }

                            if (ImGui::MenuItem("Label"))
                            {
                                auto subview = RefMake<Label>();
                                subview->SetName("Label");
                                view->AddSubview(subview);
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
                        DrawViewNodes(subviews, selections);
                        ImGui::TreePop();
                    }
                }
            }
            if (node_clicked != -1)
            {
                uint32_t node_id = views[node_clicked]->GetId();
                if (ImGui::GetIO().KeyCtrl)
                {
                    if (selections.Contains(node_id))
                    {
                        selections.Remove(node_id);
                    }
                    else
                    {
                        selections.Add(node_id);
                    }
                }
                else
                {
                    selections.Clear();
                    selections.Add(node_id);
                }
            }
        }
    };
}
