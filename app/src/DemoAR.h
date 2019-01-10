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
        Ref<MeshRenderer> m_sphere;
        
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
            m_sphere.reset();
            
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
                
#if VR_VULKAN
                String vs = R"(
UniformBuffer(0, 0) uniform UniformBuffer00
{
    mat4 u_display_matrix;
} buf_0_0;
                
Input(0) vec3 a_pos;
Input(2) vec2 a_uv;

Output(0) vec2 v_uv;

void main()
{
    gl_Position = vec4(a_pos, 1.0) * buf_0_0.u_display_matrix;
    v_uv = a_uv;
    
    vulkan_convert();
}
)";
                String fs = R"(
precision highp float;

UniformTexture(0, 1) uniform sampler2D u_texture_y;
UniformTexture(0, 2) uniform sampler2D u_texture_uv;

Input(0) vec2 v_uv;

Output(0) vec4 o_frag;

void main()
{
    vec4 y = texture(u_texture_y, v_uv);
    vec4 uv = texture(u_texture_uv, v_uv);
    vec4 yuv = vec4(y.r, uv.rg, 1.0);
    
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
                    "",
                    Vector<String>(),
                    vs,
                    "",
                    Vector<String>(),
                    fs,
                    render_state);
#elif VR_GLES
                String vs = R"(
uniform mat4 u_display_matrix;

in vec3 a_pos;
in vec2 a_uv;

out vec2 v_uv;

void main()
{
    gl_Position = vec4(a_pos, 1.0) * u_display_matrix;
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
#endif
                
                m_bg_material = RefMake<Material>(shader);
                m_bg_material->SetTexture("u_texture_y", texture_y);
                m_bg_material->SetTexture("u_texture_uv", texture_uv);
                m_bg_material->SetMatrix("u_display_matrix", m_scene->GetDisplayTransform());
                
                Display::Instance()->DestroyCamera(m_clear_camera);
                m_clear_camera = nullptr;
                
                m_bg_camera = Display::Instance()->CreateBlitCamera(
                    0,
                    m_bg_material,
                    CameraClearFlags::ColorAndDepth);

                m_scene_camera = Display::Instance()->CreateCamera();
                m_scene_camera->SetClearFlags(CameraClearFlags::Nothing);
                m_scene_camera->SetDepth(1);
                
                m_ui_camera->SetDepth(2);
                
                // test cube
                render_state = RenderState();
                render_state.cull = RenderState::Cull::Front;
#if VR_VULKAN
                shader = RefMake<Shader>(
                    "",
                    Vector<String>({ "Diffuse.vs" }),
                    "",
                    "",
                    Vector<String>({ "Diffuse.fs" }),
                    "",
                    render_state);
#elif VR_GLES
                shader = RefMake<Shader>(
                    "",
                    Vector<String>({ "Diffuse.100.vs" }),
                    "",
                    "",
                    Vector<String>({ "Diffuse.100.fs" }),
                    "",
                    render_state);
#endif
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
#if VR_VULKAN
                vs = R"(
UniformBuffer(0, 0) uniform UniformBuffer00
{
    mat4 u_view_matrix;
    mat4 u_projection_matrix;
} buf_0_0;
                
UniformBuffer(1, 0) uniform UniformBuffer10
{
    mat4 u_model_matrix;
} buf_1_0;

Input(0) vec3 a_pos;
Input(4) vec3 a_normal;

Output(0) vec3 v_pos;
Output(1) vec3 v_normal;
                
void main()
{
    vec4 pos_world = vec4(a_pos, 1.0) * buf_1_0.u_model_matrix;
    gl_Position = pos_world * buf_0_0.u_view_matrix * buf_0_0.u_projection_matrix;
    v_pos = pos_world.xyz;
    v_normal = normalize((vec4(a_normal, 0) * buf_1_0.u_model_matrix).xyz);
    
    vulkan_convert();
}
)";
                fs = R"(
precision highp float;

UniformTexture(0, 1) uniform samplerCube u_texture;

UniformBuffer(0, 2) uniform UniformBuffer02
{
    vec4 u_camera_pos;
} buf_0_2;
                
Input(0) vec3 v_pos;
Input(1) vec3 v_normal;

Output(0) vec4 o_frag;

void main()
{
    vec3 v = normalize(v_pos - buf_0_2.u_camera_pos.xyz);
    vec3 n = normalize(v_normal);
    vec3 r = reflect(v, n);
    vec4 c = texture(u_texture, r);
    o_frag = c.bgra; // bgra -> rgba
}
)";
                shader = RefMake<Shader>(
                    "",
                    Vector<String>(),
                    vs,
                    "",
                    Vector<String>(),
                    fs,
                    render_state);
#elif VR_GLES
                vs = R"(
uniform mat4 u_view_matrix;
uniform mat4 u_projection_matrix;
uniform mat4 u_model_matrix;
                
in vec3 a_pos;
in vec3 a_normal;

out vec3 v_pos;
out vec3 v_normal;

void main()
{
    vec4 pos_world = vec4(a_pos, 1.0) * u_model_matrix;
    gl_Position = pos_world * u_view_matrix * u_projection_matrix;
    v_pos = pos_world.xyz;
    v_normal = normalize((vec4(a_normal, 0) * u_model_matrix).xyz);
}
)";
                fs = R"(
precision highp float;

uniform samplerCube u_texture;
uniform vec4 u_camera_pos;

in vec3 v_pos;
in vec3 v_normal;

out vec4 o_frag;
                
void main()
{
    vec3 v = normalize(v_pos - u_camera_pos.xyz);
    vec3 n = normalize(v_normal);
    vec3 r = reflect(v, n);
    vec4 c = texture(u_texture, r);
    o_frag = c.bgra; // bgra -> rgba
}
)";
                shader = RefMake<Shader>(
                    "#version 300 es",
                    Vector<String>(),
                    vs,
                    "#version 300 es",
                    Vector<String>(),
                    fs,
                    render_state);
#endif
                
                auto sphere = Mesh::LoadFromFile(Application::Instance()->GetDataPath() + "/Library/unity default resources.Sphere.mesh");
                
                material = RefMake<Material>(shader);
                material->SetTexture("u_texture", Texture::GetSharedCubemap());
                
                renderer = RefMake<MeshRenderer>();
                renderer->SetMaterial(material);
                renderer->SetMesh(sphere);
                m_scene_camera->AddRenderer(renderer);
                
                renderer->SetLocalPosition(camera_pos + camera_rot * Vector3(-0.6f, 0, 1) * 0.2f);
                renderer->SetLocalScale(Vector3(1, 1, 1) * 0.1f);
                
                m_sphere = renderer;
            }
            else
            {
                m_bg_material->SetTexture("u_texture_y", texture_y);
                m_bg_material->SetTexture("u_texture_uv", texture_uv);
                m_bg_material->SetMatrix("u_display_matrix", m_scene->GetDisplayTransform());
                
                if (m_scene->GetEnvironmentTexture())
                {
                    m_sphere->GetMaterial()->SetTexture("u_texture", m_scene->GetEnvironmentTexture());
                }
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
