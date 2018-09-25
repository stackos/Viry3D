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

#define PROJECTION_MATRIX_EXTERNAL 0

namespace Viry3D
{
    class DemoAR : public DemoMesh
    {
    public:
        ARScene* m_scene = nullptr;
        Camera* m_clear_camera = nullptr;
        Camera* m_bg_camera = nullptr;
        Camera* m_scene_camera = nullptr;
        Ref<Material> m_bg_material;
        Rect m_viewport;
        
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
            this->InitLight();
        }
        
        virtual void Done()
        {
            if (m_scene_camera)
            {
                Display::Instance()->DestroyCamera(m_scene_camera);
                m_scene_camera = nullptr;
            }
            
            if (m_bg_camera)
            {
                Display::Instance()->DestroyCamera(m_bg_camera);
                m_bg_camera = nullptr;
            }
            
            if (m_clear_camera)
            {
                Display::Instance()->DestroyCamera(m_clear_camera);
                m_clear_camera = nullptr;
            }
            
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
                
                Display::Instance()->DestroyCamera(m_clear_camera);
                m_clear_camera = nullptr;
                
                m_bg_camera = Display::Instance()->CreateBlitCamera(
                    0,
                    Ref<Texture>(),
                    m_bg_material,
                    "",
                    CameraClearFlags::ColorAndDepth);

                m_scene_camera = Display::Instance()->CreateCamera();
                m_scene_camera->SetClearFlags(CameraClearFlags::Nothing);
                m_scene_camera->SetDepth(1);
                
                m_ui_camera->SetDepth(2);
                
                // test cube
                render_state = RenderState();
                render_state.cull = RenderState::Cull::Front;
                shader = RefMake<Shader>(
                    "",
                    Vector<String>({ "Diffuse.100.vs.in" }),
                    "",
                    "",
                    Vector<String>({ "Diffuse.100.fs.in" }),
                    "",
                    render_state);
                auto cube = Mesh::LoadFromFile(Application::Instance()->GetDataPath() + "/Library/unity default resources.Cube.mesh");
                
                auto material = RefMake<Material>(shader);
                material->SetTexture("u_texture", Texture::GetSharedWhiteTexture());
                material->SetVector("u_uv_scale_offset", Vector4(1, 1, 0, 0));
                material->SetLightProperties(m_light);
                
                auto renderer = RefMake<MeshRenderer>();
                renderer->SetMaterial(material);
                renderer->SetMesh(cube);
                m_scene_camera->AddRenderer(renderer);
                
                const auto& camera_pos = m_scene->GetCameraPosition();
                const auto& camera_rot = m_scene->GetCameraRotation();
                renderer->SetLocalPosition(camera_pos + camera_rot * Vector3(0, 0, 1) * 0.2f);
                renderer->SetLocalScale(Vector3(1, 1, 1) * 0.1f);
                
                // test sphere
                auto sphere = Mesh::LoadFromFile(Application::Instance()->GetDataPath() + "/Library/unity default resources.Sphere.mesh");
                renderer = RefMake<MeshRenderer>();
                renderer->SetMaterial(material);
                renderer->SetMesh(sphere);
                m_scene_camera->AddRenderer(renderer);
                
                renderer->SetLocalPosition(camera_pos + camera_rot * Vector3(-0.6f, 0, 1) * 0.2f);
                renderer->SetLocalScale(Vector3(1, 1, 1) * 0.1f);
            }
            else
            {
                m_bg_material->SetTexture("u_texture_y", texture_y);
                m_bg_material->SetTexture("u_texture_uv", texture_uv);
                m_bg_material->SetMatrix("u_display_matrix", m_scene->GetDisplayTransform());
            }
            
            this->UpdateCamera(texture_y->GetWidth(), texture_y->GetHeight());
            
            DemoMesh::Update();
        }
        
        void UpdateCamera(int texture_w, int texture_h)
        {
            if (Display::Instance()->GetWidth() < Display::Instance()->GetHeight())
            {
                if (texture_w > texture_h)
                {
                    std::swap(texture_w, texture_h);
                }
            }
            else
            {
                if (texture_w < texture_h)
                {
                    std::swap(texture_w, texture_h);
                }
            }
            
            if (texture_w != Display::Instance()->GetWidth() || texture_h != Display::Instance()->GetHeight())
            {
                float aspect_texture = texture_w / (float) texture_h;
                float aspect_screen = Display::Instance()->GetWidth() / (float) Display::Instance()->GetHeight();
                
                int w;
                int h;
                if (aspect_texture > aspect_screen)
                {
                    w = Display::Instance()->GetWidth();
                    h = texture_h *  w / Display::Instance()->GetWidth();
                    
                    m_viewport.x = 0;
                    m_viewport.width = 1.0f;
                    m_viewport.y = (Display::Instance()->GetHeight() - h) / 2 / (float) Display::Instance()->GetHeight();
                    m_viewport.height = h / (float) Display::Instance()->GetHeight();
                }
                else
                {
                    h = Display::Instance()->GetHeight();
                    w = texture_w *  h / Display::Instance()->GetHeight();
                    
                    m_viewport.x = (Display::Instance()->GetWidth() - w) / 2 / (float) Display::Instance()->GetWidth();
                    m_viewport.width = w / (float) Display::Instance()->GetWidth();
                    m_viewport.y = 0;
                    m_viewport.height = 1.0f;
                }

                m_bg_camera->SetViewportRect(m_viewport);
                m_scene_camera->SetViewportRect(m_viewport);
            }
            
            m_scene_camera->SetLocalPosition(m_scene->GetCameraPosition());
            m_scene_camera->SetLocalRotation(m_scene->GetCameraRotation());
            m_scene_camera->SetViewMatrixExternal(m_scene->GetCameraViewMatrix());
            
#if PROJECTION_MATRIX_EXTERNAL
            m_scene_camera->SetProjectionMatrixExternal(m_scene->GetCameraProjectionMatrix());
#else
            m_scene_camera->SetFieldOfView(m_scene->GetCameraFov());
            m_scene_camera->SetNearClip(m_scene->GetCameraNear());
            m_scene_camera->SetFarClip(m_scene->GetCameraFar());
#endif
        }
    };
}
