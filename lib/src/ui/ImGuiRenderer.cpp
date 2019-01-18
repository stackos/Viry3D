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

#include "ImGuiRenderer.h"
#include "imgui/imgui.h"
#include "time/Time.h"

namespace Viry3D
{
    ImGuiRenderer::ImGuiRenderer()
    {
        ImGui::CreateContext();
    }

    ImGuiRenderer::~ImGuiRenderer()
    {
        ImGui::DestroyContext();
    }

    Ref<BufferObject> ImGuiRenderer::GetVertexBuffer() const
    {
        Ref<BufferObject> buffer;

        return buffer;
    }

    Ref<BufferObject> ImGuiRenderer::GetIndexBuffer() const
    {
        Ref<BufferObject> buffer;

        return buffer;
    }

    void ImGuiRenderer::UpdateDrawBuffer()
    {
        
    }

    void ImGuiRenderer::Update()
    {
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float) Display::Instance()->GetWidth(), (float) Display::Instance()->GetHeight());
        io.DeltaTime = Time::GetDeltaTime();

        unsigned char* font_tex_pixels;
        int font_tex_width;
        int font_tex_height;
        io.Fonts->GetTexDataAsRGBA32(&font_tex_pixels, &font_tex_width, &font_tex_height);

        ImGui::NewFrame();
        if (m_draw)
        {
            m_draw();
        }
        ImGui::Render();

        ImDrawData* draw_data = ImGui::GetDrawData();

        Renderer::Update();
    }
}
