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

#include "graphics/Display.h"
#include "math/Rect.h"
#include "container/Vector.h"
#include "container/Map.h"
#include "string/String.h"
#include "memory/ByteBuffer.h"

namespace Viry3D
{
    class Object;
    class Node;
    class Camera;
    class ImGuiRenderer;
    class Texture;
    class View;

    class CanvasEditor
    {
    public:
        void Init();
        void Done();
        void Update();

        void InitUI();
        void OnGUI();
        void ShowMainMenuBar();
        bool BeginMainWindow(const char* name, float x, float y, float w, float h, bool resize = true);
        void EndMainWindow();
        void ShowSceneWindow();
        void ShowPropertyWindow();
        void ShowViewWindow();
        void ShowConsoleWindow();

        Ref<Camera> CreateCamera();
        void DestroyCamera(const Ref<Camera>& camera);
        const Vector<Ref<Camera>>& GetCameras() const { return m_cameras; }
        Vector<uint32_t>& GetSelections() { return m_selections; }
        ByteBuffer& GetTextBuffer(const String& name);
        Ref<Object> GetSelectionObject(uint32_t id);
		String OpenFilePanel(const String& initial_path, const char* filter);

    private:
        Ref<Object> FindView(const Ref<View>& view, uint32_t id);
        Ref<Object> FindNode(const Ref<Node>& node, uint32_t id);

    private:
        Ref<Camera> m_imgui_camera;
        Ref<ImGuiRenderer> m_imgui;
        bool m_show_demo_window = false;
        Rect m_menu_rect;
        Rect m_scene_window_rect = Rect(0, 0, (float) (Display::Instance()->GetWidth() * 200 / 1280), (float) (Display::Instance()->GetHeight() * 450 / 720));
        Rect m_property_window_rect = Rect(0, 0, (float) (Display::Instance()->GetWidth() * 200 / 1280), 0);
        Rect m_view_window_rect = Rect(0, 0, 0, 0);
        Rect m_console_window_rect = Rect(0, 0, 0, 0);
        Vector<Ref<Camera>> m_cameras;
        Ref<Texture> m_view_rt;
        Ref<Texture> m_view_depth;
        Vector<uint32_t> m_selections;
        Map<String, ByteBuffer> m_text_buffer;
    };
}
