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

#include "memory/Ref.h"

namespace Viry3D
{
    class GameObject;
    class ImGuiRenderer;

    class Editor
    {
    public:
        bool IsInEditorMode() const { return m_editor_mode; }
        Ref<GameObject> GetSelectedGameObject() const { return m_selected_object.lock(); }
        void Update();

    private:
        bool m_editor_mode = false;
        WeakRef<GameObject> m_selected_object;
        WeakRef<ImGuiRenderer> m_imgui;
    };
}
