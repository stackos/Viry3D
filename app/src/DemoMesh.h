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

#include "Demo.h"
#include "Application.h"
#include "graphics/Display.h"
#include "graphics/Camera.h"
#include "graphics/Shader.h"
#include "graphics/Material.h"
#include "graphics/MeshRenderer.h"
#include "graphics/Mesh.h"
#include "graphics/Texture.h"
#include "math/Quaternion.h"
#include "time/Time.h"
#include "ui/CanvasRenderer.h"
#include "ui/Label.h"
#include "ui/Font.h"

namespace Viry3D
{
    class DemoMesh : public Demo
    {
    public:
        struct CameraParam
        {
            Vector3 pos;
            Quaternion rot;
            float fov;
            float near_clip;
            float far_clip;
        };
        CameraParam m_camera_param = {
            Vector3(0, 3, -4),
            Quaternion::Euler(30, 0, 0),
            45,
            0.3f,
            1000
        };

        Camera* m_camera;
        Vector<Ref<MeshRenderer>> m_renderers;
        Label* m_label;

        void InitMesh()
        {
            String vs = R"(
UniformBuffer(0, 0) uniform UniformBuffer00
{
	mat4 u_view_matrix;
	mat4 u_projection_matrix;
    vec4 u_uv_scale_offset;
} buf_0_0;

UniformBuffer(1, 0) uniform UniformBuffer10
{
	mat4 u_model_matrix;
} buf_1_0;

Input(0) vec4 a_pos;
Input(2) vec2 a_uv;
Input(4) vec3 a_normal;

Output(0) vec2 v_uv;
Output(1) vec3 v_normal;

void main()
{
	gl_Position = a_pos * buf_1_0.u_model_matrix * buf_0_0.u_view_matrix * buf_0_0.u_projection_matrix;
	v_uv = a_uv * buf_0_0.u_uv_scale_offset.xy + buf_0_0.u_uv_scale_offset.zw;
    v_normal = normalize((vec4(a_normal, 0) * buf_1_0.u_model_matrix).xyz);
	
    vulkan_convert();
}
)";
            String fs = R"(
precision highp float;

UniformTexture(0, 1) uniform sampler2D u_texture;

UniformBuffer(0, 2) uniform UniformBuffer02
{
    vec4 u_light_color;
    vec4 u_light_dir;
    float u_light_intensity;
} buf_0_2;

Input(0) vec2 v_uv;
Input(1) vec3 v_normal;

Output(0) vec4 o_frag;

void main()
{
    vec4 c = texture(u_texture, v_uv);
    vec3 n = normalize(v_normal);
    vec3 l = normalize(-buf_0_2.u_light_dir.xyz);
    
    float nl = max(dot(n, l), 0.0);
    vec3 diff = c.rgb * nl * buf_0_2.u_light_color.rgb * buf_0_2.u_light_intensity;

    c.rgb = diff;
    c.a = 1.0;

    o_frag = c;
}
)";
            RenderState render_state;

            auto shader = RefMake<Shader>(
                "",
                Vector<String>(),
                vs,
                "",
                Vector<String>(),
                fs,
                render_state);

            Color light_color = Color(1, 1, 1, 1);
            Vector3 light_dir = Quaternion::Euler(45, 60, 0) * Vector3(0, 0, 1);
            float light_intensity = 1.0f;

            // plane
            auto texture = Texture::LoadTexture2DFromFile(Application::Instance()->GetDataPath() + "/texture/checkflag.png", VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, true);

            Vector3 camera_forward = m_camera_param.rot * Vector3(0, 0, 1);
            Vector3 camera_up = m_camera_param.rot * Vector3(0, 1, 0);
            Matrix4x4 view = Matrix4x4::LookTo(m_camera_param.pos, camera_forward, camera_up);
            Matrix4x4 projection = Matrix4x4::Perspective(m_camera_param.fov, m_camera->GetTargetWidth() / (float) m_camera->GetTargetHeight(), m_camera_param.near_clip, m_camera_param.far_clip);

            auto material = RefMake<Material>(shader);
            material->SetTexture("u_texture", texture);
            material->SetVector("u_uv_scale_offset", Vector4(10, 10, 0, 0));
            material->SetMatrix("u_view_matrix", view);
            material->SetMatrix("u_projection_matrix", projection);
            material->SetColor("u_light_color", light_color);
            material->SetVector("u_light_dir", light_dir);
            material->SetFloat("u_light_intensity", light_intensity);

            auto cube = Mesh::LoadFromFile(Application::Instance()->GetDataPath() + "/Library/unity default resources.Cube.mesh");

            auto renderer = RefMake<MeshRenderer>();
            renderer->SetMaterial(material);
            renderer->SetMesh(cube);
            m_camera->AddRenderer(renderer);
            m_renderers.Add(renderer);

            Matrix4x4 model = Matrix4x4::Translation(Vector3(0, -0.5f, 0)) * Matrix4x4::Scaling(Vector3(10, 1, 10));
            renderer->SetInstanceMatrix("u_model_matrix", model);

            // cube
            material = RefMake<Material>(shader);
            material->SetTexture("u_texture", Texture::GetSharedWhiteTexture());
            material->SetVector("u_uv_scale_offset", Vector4(1, 1, 0, 0));
            material->SetMatrix("u_view_matrix", view);
            material->SetMatrix("u_projection_matrix", projection);
            material->SetColor("u_light_color", light_color);
            material->SetVector("u_light_dir", light_dir);
            material->SetFloat("u_light_intensity", light_intensity);

            renderer = RefMake<MeshRenderer>();
            renderer->SetMaterial(material);
            renderer->SetMesh(cube);
            m_camera->AddRenderer(renderer);
            m_renderers.Add(renderer);

            model = Matrix4x4::Translation(Vector3(-0.7f, 0.72f, -0.2f)) * Matrix4x4::Scaling(Vector3(1, 1.44f, 1));
            renderer->SetInstanceMatrix("u_model_matrix", model);

            // sphere
            auto sphere = Mesh::LoadFromFile(Application::Instance()->GetDataPath() + "/Library/unity default resources.Sphere.mesh");

            renderer = RefMake<MeshRenderer>();
            renderer->SetMaterial(material);
            renderer->SetMesh(sphere);
            m_camera->AddRenderer(renderer);
            m_renderers.Add(renderer);

            model = Matrix4x4::Translation(Vector3(0.6f, 0.5f, -0.2f));
            renderer->SetInstanceMatrix("u_model_matrix", model);

            // cylinder
            auto cylinder = Mesh::LoadFromFile(Application::Instance()->GetDataPath() + "/Library/unity default resources.Cylinder.mesh");

            renderer = RefMake<MeshRenderer>();
            renderer->SetMaterial(material);
            renderer->SetMesh(cylinder);
            m_camera->AddRenderer(renderer);
            m_renderers.Add(renderer);

            model = Matrix4x4::Translation(Vector3(-1.4f, 1.2f, -1)) * Matrix4x4::Scaling(Vector3(0.2f, 1.2f, 0.2f));
            renderer->SetInstanceMatrix("u_model_matrix", model);
        }

        void InitUI()
        {
            auto canvas = RefMake<CanvasRenderer>();
            m_camera->AddRenderer(canvas);

            auto label = RefMake<Label>();
            canvas->AddView(label);

            label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetPivot(Vector2(0, 0));
            label->SetSize(Vector2i(100, 30));
            label->SetOffset(Vector2i(40, 40));
            label->SetFont(Font::GetFont(FontType::PingFangSC));
            label->SetFontSize(28);
            label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);

            m_label = label.get();
        }

        virtual void Init()
        {
            m_camera = Display::Instance()->CreateCamera();

            this->InitMesh();
            this->InitUI();
        }

        virtual void Done()
        {
            m_renderers.Clear();

            Display::Instance()->DestroyCamera(m_camera);
            m_camera = nullptr;
        }

        virtual void Update()
        {
            m_label->SetText(String::Format("FPS:%d", Time::GetFPS()));
        }

        virtual void OnResize(int width, int height)
        {
            Matrix4x4 projection = Matrix4x4::Perspective(m_camera_param.fov, m_camera->GetTargetWidth() / (float) m_camera->GetTargetHeight(), m_camera_param.near_clip, m_camera_param.far_clip);
            for (auto i : m_renderers)
            {
                i->GetMaterial()->SetMatrix("u_projection_matrix", projection);
            }
        }
    };
}
