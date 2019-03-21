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
