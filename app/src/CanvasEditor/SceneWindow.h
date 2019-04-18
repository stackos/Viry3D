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
#include "graphics/Camera.h"
#include "ui/CanvasRenderer.h"
#include "ui/Sprite.h"
#include "ui/Label.h"
#include "Resources.h"

namespace Viry3D
{
    class SceneWindow
    {
    public:
        static void OnGUI(CanvasEditor* editor)
        {
            Vector<uint32_t>& selections = editor->GetSelections();

            if (ImGui::BeginPopupContextWindow())
            {
                if (ImGui::BeginMenu("Create"))
                {
                    if (ImGui::MenuItem("Camera"))
                    {
                        Ref<Camera> camera = editor->CreateCamera();
                        camera->SetName("Camera");
                    }

                    ImGui::EndMenu();
                }

                ImGui::EndPopup();
            }

            Vector<Ref<Camera>> cameras = editor->GetCameras();
            int node_clicked = -1;
            for (int i = 0; i < cameras.Size(); ++i)
            {
                Ref<Camera> camera = cameras[i];
                Vector<Ref<Node>> nodes = camera->GetNodes();

                ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (selections.Contains(camera->GetId()) ? ImGuiTreeNodeFlags_Selected : 0);
                bool leaf = (nodes.Size() == 0);

                if (leaf)
                {
                    node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                }

                bool node_open = ImGui::TreeNodeEx((void*) (intptr_t) i, node_flags, camera->GetName().CString());
                if (ImGui::IsItemClicked())
                {
                    node_clicked = i;
                }

                String id = String::Format("camera context menu id %d", camera->GetId());
                if (ImGui::BeginPopupContextItem(id.CString()))
                {
                    if (ImGui::BeginMenu("Create"))
                    {
						if (ImGui::MenuItem("Node"))
						{
							auto node = RefMake<Node>();
							node->SetName("Node");
							camera->AddNode(node);
						}

						if (ImGui::MenuItem("Canvas"))
						{
							auto canvas = RefMake<CanvasRenderer>(FilterMode::Nearest);
							canvas->SetName("Canvas");
							camera->AddNode(canvas);
						}

                        ImGui::EndMenu();
                    }

					if (ImGui::BeginMenu("Load"))
					{
						if (ImGui::MenuItem("Node"))
						{
							String initial_path = Application::Instance()->GetDataPath();
							String node_path = editor->OpenFilePanel(initial_path, "GameObject\0*.go\0");
							if (node_path.Size() > 0)
							{
								int find = node_path.LastIndexOf("/Assets/");
								if (find >= 0)
								{
									node_path = node_path.Substring(find + String("/Assets/").Size());
									Ref<Node> node = Resources::LoadNode(node_path);
									if (node)
									{
										camera->AddNode(node);
									}
								}
							}
						}

						ImGui::EndMenu();
					}

                    if (ImGui::MenuItem("Destroy"))
                    {
                        editor->DestroyCamera(camera);

                        if (selections.Contains(camera->GetId()))
                        {
                            selections.Remove(camera->GetId());
                        }

                        for (int j = 0; j < nodes.Size(); ++j)
                        {
                            RemoveNodeSelection(nodes[j], selections);
                        }
                    }

                    ImGui::EndPopup();
                }

                if (!leaf)
                {
                    if (node_open)
                    {
                        DrawNodes(nodes, selections, camera, Ref<Node>());
                        ImGui::TreePop();
                    }
                }
            }
            if (node_clicked != -1)
            {
                uint32_t node_id = cameras[node_clicked]->GetId();
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

        static void DrawNodes(const Vector<Ref<Node>>& nodes, Vector<uint32_t>& selections, const Ref<Camera>& camera, const Ref<Node>& parent)
        {
            int node_clicked = -1;
            for (int i = 0; i < nodes.Size(); ++i)
            {
                const Ref<Node>& node = nodes[i];
                int child_count = node->GetChildCount();
                Ref<CanvasRenderer> canvas = RefCast<CanvasRenderer>(node);

                ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (selections.Contains(node->GetId()) ? ImGuiTreeNodeFlags_Selected : 0);
                bool leaf = true;
                
                if (canvas)
                {
                    leaf = (canvas->GetViews().Size() == 0);
                }
                else
                {
                    leaf = (child_count == 0);
                }

                if (leaf)
                {
                    node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                }

                bool node_open = ImGui::TreeNodeEx((void*) (intptr_t) i, node_flags, node->GetName().CString());
                if (ImGui::IsItemClicked())
                {
                    node_clicked = i;
                }

                if (canvas)
                {
                    String id = String::Format("canvas context menu id %d", canvas->GetId());
                    if (ImGui::BeginPopupContextItem(id.CString()))
                    {
                        if (ImGui::BeginMenu("Create"))
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
                                view->SetText("Label");
                                canvas->AddView(view);
                            }

                            ImGui::EndMenu();
                        }

                        if (ImGui::MenuItem("Destroy"))
                        {
                            if (!parent)
                            {
                                camera->RemoveNode(node);
                            }

                            if (selections.Contains(node->GetId()))
                            {
                                selections.Remove(node->GetId());
                            }

                            RemoveNodeSelection(node, selections);
                        }

                        ImGui::EndPopup();
                    }

                    if (!leaf)
                    {
                        if (node_open)
                        {
                            DrawViewNodes(canvas->GetViews(), selections);
                            ImGui::TreePop();
                        }
                    }
                }
                else
                {
					String id = String::Format("node context menu id %d", node->GetId());
					if (ImGui::BeginPopupContextItem(id.CString()))
					{
						if (ImGui::BeginMenu("Create"))
						{
							if (ImGui::MenuItem("Node"))
							{
								auto child_node = RefMake<Node>();
								child_node->SetName("Node");
								Node::SetParent(child_node, node);
							}

							ImGui::EndMenu();
						}

						if (ImGui::MenuItem("Destroy"))
						{
							if (!parent)
							{
								camera->RemoveNode(node);
							}
							else
							{
								Node::SetParent(node, Ref<Node>());

								Ref<Renderer> renderer = RefCast<Renderer>(node);
								if (renderer)
								{
									camera->RemoveRenderer(renderer);
								}
							}

							if (selections.Contains(node->GetId()))
							{
								selections.Remove(node->GetId());
							}

							RemoveNodeSelection(node, selections);
						}

						ImGui::EndPopup();
					}

                    if (!leaf)
                    {
                        if (node_open)
                        {
                            Vector<Ref<Node>> children(child_count);
                            for (int j = 0; j < children.Size(); j++)
                            {
                                children[j] = node->GetChild(j);
                            }
                            DrawNodes(children, selections, camera, node);
                            ImGui::TreePop();
                        }
                    }
                }
            }
            if (node_clicked != -1)
            {
                uint32_t node_id = nodes[node_clicked]->GetId();
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
                            subview->SetText("Label");
                            view->AddSubview(subview);
                        }

                        ImGui::EndMenu();
                    }

                    if (ImGui::MenuItem("Destroy"))
                    {
                        if (view->GetParentView())
                        {
                            view->GetParentView()->RemoveSubview(view);
                        }
                        else if (view->GetCanvas())
                        {
                            view->GetCanvas()->RemoveView(view);
                        }

                        if (selections.Contains(view->GetId()))
                        {
                            selections.Remove(view->GetId());
                        }

                        RemoveViewSelection(view, selections);
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

        static void RemoveNodeSelection(const Ref<Node>& node, Vector<uint32_t>& selections)
        {
            if (selections.Contains(node->GetId()))
            {
                selections.Remove(node->GetId());
            }

            Ref<CanvasRenderer> canvas = RefCast<CanvasRenderer>(node);
            if (canvas)
            {
                const Vector<Ref<View>>& views = canvas->GetViews();
                for (int i = 0; i < views.Size(); ++i)
                {
                    RemoveViewSelection(views[i], selections);
                }
            }
            else
            {
                for (int i = 0; i < node->GetChildCount(); ++i)
                {
                    RemoveNodeSelection(node->GetChild(i), selections);
                }
            }
        }

        static void RemoveViewSelection(const Ref<View>& view, Vector<uint32_t>& selections)
        {
            if (selections.Contains(view->GetId()))
            {
                selections.Remove(view->GetId());
            }

            for (int i = 0; i < view->GetSubviewCount(); ++i)
            {
                RemoveViewSelection(view->GetSubview(i), selections);
            }
        }
    };
}
