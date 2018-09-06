/*
* Viry3D
* Copyright 2014-2018 by Stack - stackos@qq.com
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

#include "DemoMesh.h"
#include "ios/ARScene.h"

namespace Viry3D
{
    class DemoAR : public DemoMesh
    {
    public:
        ARScene* m_scene = nullptr;
        Camera* m_clear_camera = nullptr;
        Camera* m_bg_camera = nullptr;
        Ref<Material> m_bg_material;

        void InitAR()
        {
            m_scene = new ARScene();
            m_scene->Run();
            m_clear_camera = Display::Instance()->CreateCamera();
        }
        
        virtual void Init()
        {
            this->InitUI();
            this->InitAR();
        }
        
        virtual void Done()
        {
            Display::Instance()->DestroyCamera(m_bg_camera);
            m_bg_camera = nullptr;
            
            delete m_scene;
            m_scene = nullptr;
            
            DemoMesh::Done();
        }

        virtual void Update()
        {
            auto texture_y = m_scene->GetCameraTextureY();
            auto texture_uv = m_scene->GetCameraTextureUV();
            if (!texture_y || !texture_uv)
            {
                return;
            }
            
            if (m_bg_camera == nullptr)
            {
                RenderState render_state;
                render_state.cull = RenderState::Cull::Off;
                render_state.zTest = RenderState::ZTest::Off;
                render_state.zWrite = RenderState::ZWrite::Off;
                
                String vs = R"(
uniform mat4 u_display_matrix;

in vec4 a_pos;
in vec2 a_uv;

out vec2 v_uv;

void main()
{
    gl_Position = a_pos * u_display_matrix;
    v_uv = a_uv;
}
)";
                String fs = R"(
precision highp float;
                
uniform sampler2D u_texture_y;
uniform sampler2D u_texture_uv;

in vec2 v_uv;

out vec4 o_frag;

void main()
{
    vec4 y = texture(u_texture_y, v_uv);
    vec4 uv = texture(u_texture_uv, v_uv);
    vec4 yuv = vec4(y.r, uv.ra, 1.0);
    
    mat4 to_rgb = mat4(
        vec4(+1.0000f, +1.0000f, +1.0000f, +0.0000f),
        vec4(+0.0000f, -0.3441f, +1.7720f, +0.0000f),
        vec4(+1.4020f, -0.7141f, +0.0000f, +0.0000f),
        vec4(-0.7010f, +0.5291f, -0.8860f, +1.0000f)
    );
    o_frag = to_rgb * yuv;
}
)";
                
                auto shader = RefMake<Shader>(
                    "#version 300 es",
                    Vector<String>(),
                    vs,
                    "#version 300 es",
                    Vector<String>(),
                    fs,
                    render_state);
                
                m_bg_material = RefMake<Material>(shader);
                m_bg_material->SetTexture("u_texture_y", texture_y);
                m_bg_material->SetTexture("u_texture_uv", texture_uv);
                m_bg_material->SetMatrix("u_display_matrix", m_scene->GetDisplayTransform());
                
                m_bg_camera = Display::Instance()->CreateBlitCamera(
                    0,
                    Ref<Texture>(),
                    m_bg_material,
                    "",
                    CameraClearFlags::ColorAndDepth);
                
                Display::Instance()->DestroyCamera(m_clear_camera);
                m_clear_camera = nullptr;
            }
            else
            {
                m_bg_material->SetTexture("u_texture_y", texture_y);
                m_bg_material->SetTexture("u_texture_uv", texture_uv);
                m_bg_material->SetMatrix("u_display_matrix", m_scene->GetDisplayTransform());
            }
            
            DemoMesh::Update();
        }
    };
}
