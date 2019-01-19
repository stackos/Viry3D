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

#include "graphics/Renderer.h"
#include "Action.h"

namespace Viry3D
{
    class Mesh;

    class ImGuiRenderer : public Renderer
    {
    public:
        ImGuiRenderer();
        virtual ~ImGuiRenderer();
        virtual Ref<BufferObject> GetVertexBuffer() const;
        virtual Ref<BufferObject> GetIndexBuffer() const;
        virtual void Update();
        void SetDrawAction(Action draw) { m_draw = draw; }

    protected:
        virtual void UpdateDrawBuffer();

    private:
        void CreateMaterial();
        
    private:
        Action m_draw;
        Ref<Texture> m_font_texture;
        Ref<Mesh> m_mesh;
    };
}
