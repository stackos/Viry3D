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

namespace Viry3D
{
    class PropertyWindow
    {
    public:
        static bool InputText(CanvasEditor* editor, const char* label, const String& buffer_name, String& target)
        {
            ByteBuffer& name_buffer = editor->GetTextBuffer(buffer_name);
            if (target.Size() < name_buffer.Size())
            {
                Memory::Copy(name_buffer.Bytes(), target.CString(), target.Size());
                name_buffer[target.Size()] = 0;
            }
            if (ImGui::InputText(label, (char*) name_buffer.Bytes(), name_buffer.Size()))
            {
                target = String((char*) name_buffer.Bytes(), (int) strlen((char*) name_buffer.Bytes()));
                return true;
            }
            return false;
        }

        static bool DrawAlignmentCombo(const char* label, int* alignment)
        {
            bool value_changed = false;

            String alignment_str;
            switch (*alignment & 0x0000000f)
            {
                case ViewAlignment::Left:
                    alignment_str += "Left";
                    break;
                case ViewAlignment::HCenter:
                    alignment_str += "HCenter";
                    break;
                case ViewAlignment::Right:
                    alignment_str += "Right";
                    break;
            }
            alignment_str += "-";
            switch (*alignment & 0x000000f0)
            {
                case ViewAlignment::Top:
                    alignment_str += "Top";
                    break;
                case ViewAlignment::VCenter:
                    alignment_str += "VCenter";
                    break;
                case ViewAlignment::Bottom:
                    alignment_str += "Bottom";
                    break;
            }
            if (ImGui::BeginCombo(label, alignment_str.CString()))
            {
                bool select = *alignment & ViewAlignment::Left;
                if (ImGui::Selectable("Left", &select))
                {
                    *alignment = (*alignment & 0xfffffff0) | ViewAlignment::Left;
                    value_changed = true;
                }

                select = *alignment & ViewAlignment::HCenter;
                if (ImGui::Selectable("HCenter", &select))
                {
                    *alignment = (*alignment & 0xfffffff0) | ViewAlignment::HCenter;
                    value_changed = true;
                }

                select = *alignment & ViewAlignment::Right;
                if (ImGui::Selectable("Right", &select))
                {
                    *alignment = (*alignment & 0xfffffff0) | ViewAlignment::Right;
                    value_changed = true;
                }

                select = *alignment & ViewAlignment::Top;
                if (ImGui::Selectable("Top", &select))
                {
                    *alignment = (*alignment & 0xffffff0f) | ViewAlignment::Top;
                    value_changed = true;
                }

                select = *alignment & ViewAlignment::VCenter;
                if (ImGui::Selectable("VCenter", &select))
                {
                    *alignment = (*alignment & 0xffffff0f) | ViewAlignment::VCenter;
                    value_changed = true;
                }

                select = *alignment & ViewAlignment::Bottom;
                if (ImGui::Selectable("Bottom", &select))
                {
                    *alignment = (*alignment & 0xffffff0f) | ViewAlignment::Bottom;
                    value_changed = true;
                }

                ImGui::EndCombo();
            }

            return value_changed;
        }

        static void OnGUI(CanvasEditor* editor)
        {
            Camera* camera = editor->GetCamera();
            Vector<uint32_t>& selections = editor->GetSelections();
            
            if (selections.Size() == 1)
            {
                Ref<Object> obj = editor->GetSelectionObject(selections[0]);

                String name = obj->GetName();
                if (InputText(editor, "Name", "name", name))
                {
                    obj->SetName(name);
                }

                Ref<View> view = RefCast<View>(obj);
                if (view)
                {
                    Color color = view->GetColor();
                    if (ImGui::ColorEdit4("Color", (float*) &color))
                    {
                        view->SetColor(color);
                    }

                    int alignment = view->GetAlignment();
                    if (DrawAlignmentCombo("Alignment", &alignment))
                    {
                        view->SetAlignment(alignment);
                    }

                    Vector2 pivot = view->GetPivot();
                    if (ImGui::InputFloat2("Pivot", (float*) &pivot, 3))
                    {
                        view->SetPivot(pivot);
                    }

                    Vector2i size = view->GetSize();
                    bool size_w_fill_parent = (size.x == VIEW_SIZE_FILL_PARENT);
                    bool size_h_fill_parent = (size.y == VIEW_SIZE_FILL_PARENT);
                    if (ImGui::Checkbox("WidthFillParent", &size_w_fill_parent))
                    {
                        if (size_w_fill_parent)
                        {
                            size.x = VIEW_SIZE_FILL_PARENT;
                        }
                        else
                        {
                            size.x = 100;
                        }
                        view->SetSize(size);
                    }
                    if (!size_w_fill_parent)
                    {
                        if (ImGui::InputInt("Width", &size.x))
                        {
                            view->SetSize(size);
                        }
                    }
                    if (ImGui::Checkbox("HeightFillParent", &size_h_fill_parent))
                    {
                        if (size_h_fill_parent)
                        {
                            size.y = VIEW_SIZE_FILL_PARENT;
                        }
                        else
                        {
                            size.y = 100;
                        }
                        view->SetSize(size);
                    }
                    if (!size_h_fill_parent)
                    {
                        if (ImGui::InputInt("Height", &size.y))
                        {
                            view->SetSize(size);
                        }
                    }

                    Vector2i offset = view->GetOffset();
                    if (ImGui::InputInt2("Offset", (int*) &offset))
                    {
                        view->SetOffset(offset);
                    }

                    Vector4 margin = view->GetMargin();
                    int margin_i[4] = { (int) margin.x, (int) margin.y, (int) margin.z, (int) margin.w };
                    if (ImGui::InputInt4("Margin", margin_i))
                    {
                        margin = Vector4((float) margin_i[0], (float) margin_i[1], (float) margin_i[2], (float) margin_i[3]);
                        view->SetMargin(margin);
                    }
                }
            }
            else if (selections.Size() > 1)
            {
                ImGui::Text("Multi-node selected");
            }
            else
            {
                ImGui::Text("No selection");
            }
        }
    };
}
