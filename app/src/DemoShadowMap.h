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

namespace Viry3D
{
    class DemoShadowMap : public DemoMesh
    {
    public:
        struct LightParam
        {
            Vector3 pos;
            Quaternion rot;
            Color color;
            float intensity;
            float ortho_size;
            float near_clip;
            float far_clip;
        };
        LightParam m_light_param = {
            Vector3(0, 0, 0),
            Quaternion::Euler(45, 60, 0),
            Color(1, 1, 1, 1),
            1.0f,
            1.5f,
            -5,
            5
        };

        Camera* m_shadow_camera;
        Vector<Ref<MeshRenderer>> m_shadow_renderers;
        Ref<Texture> m_shadow_texture;
        Matrix4x4 m_light_view_projection_matrix;

        void InitShadowCaster()
        {
            m_camera->SetDepth(1);

            const int shadow_map_size = 1024;
            m_shadow_texture = Texture::CreateRenderTexture(
                    shadow_map_size,
                    shadow_map_size,
                    Display::Instance()->ChooseFormatSupported(
                            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_X8_D24_UNORM_PACK32, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT),
                    true,
                    VK_FILTER_LINEAR,
                    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

            m_shadow_camera = Display::Instance()->CreateCamera();
            m_shadow_camera->SetDepth(0);
            m_shadow_camera->SetClearFlags(CameraClearFlags::Depth);
            m_shadow_camera->SetRenderTarget(Ref<Texture>(), m_shadow_texture);

            String vs = R"(
UniformBuffer(0, 0) uniform UniformBuffer00
{
	mat4 u_view_matrix;
	mat4 u_projection_matrix;
} buf_0_0;

UniformBuffer(1, 0) uniform UniformBuffer10
{
	mat4 u_model_matrix;
} buf_1_0;

Input(0) vec4 a_pos;

void main()
{
	gl_Position = a_pos * buf_1_0.u_model_matrix * buf_0_0.u_view_matrix * buf_0_0.u_projection_matrix;

    vulkan_convert();
}
)";
            String fs = R"(
precision highp float;

void main()
{
}
)";
            RenderState render_state;
            render_state.cull = RenderState::Cull::Front;

            auto shader = RefMake<Shader>(
                    "",
                    Vector<String>(),
                    vs,
                    "",
                    Vector<String>(),
                    fs,
                    render_state);
            auto material = RefMake<Material>(shader);

            Vector3 light_forward = m_light_param.rot * Vector3(0, 0, 1);
            Vector3 light_up = m_light_param.rot * Vector3(0, 1, 0);
            Matrix4x4 view = Matrix4x4::LookTo(m_light_param.pos, light_forward, light_up);

            int target_width = shadow_map_size;
            int target_height = shadow_map_size;
            float ortho_size = m_light_param.ortho_size;
            float top = ortho_size;
            float bottom = -ortho_size;
            float plane_h = ortho_size * 2;
            float plane_w = plane_h * target_width / target_height;
            Matrix4x4 projection = Matrix4x4::Ortho(-plane_w / 2, plane_w / 2, bottom, top, m_light_param.near_clip, m_light_param.far_clip);
            m_light_view_projection_matrix = projection * view;

            material->SetMatrix("u_view_matrix", view);
            material->SetMatrix("u_projection_matrix", projection);

            m_shadow_renderers.Resize(m_renderers.Size());
            for (int i = 0; i < m_shadow_renderers.Size(); ++i)
            {
                auto renderer = RefMake<MeshRenderer>();
                renderer->SetMaterial(material);
                renderer->SetMesh(m_renderers[i]->GetMesh(), m_renderers[i]->GetSubmesh());
                renderer->SetInstanceMatrix("u_model_matrix", *m_renderers[i]->GetInstanceMatrix("u_model_matrix"));

                m_shadow_renderers[i] = renderer;
                m_shadow_camera->AddRenderer(renderer);
            }

            Display::Instance()->CreateBlitCamera(2, m_shadow_texture, Ref<Material>(), "", CameraClearFlags::Nothing, Rect(0.75f, 0, 0.25f, 0.25f));
        }

        void InitShadowReciever()
        {
            String vs = R"(
UniformBuffer(0, 0) uniform UniformBuffer00
{
	mat4 u_view_matrix;
	mat4 u_projection_matrix;
    vec4 u_uv_scale_offset;
    mat4 u_light_view_projection_matrix;
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
Output(2) vec4 v_pos_light_proj;

void main()
{
	gl_Position = a_pos * buf_1_0.u_model_matrix * buf_0_0.u_view_matrix * buf_0_0.u_projection_matrix;
	v_uv = a_uv * buf_0_0.u_uv_scale_offset.xy + buf_0_0.u_uv_scale_offset.zw;
    v_normal = normalize((vec4(a_normal, 0) * buf_1_0.u_model_matrix).xyz);
	v_pos_light_proj = a_pos * buf_1_0.u_model_matrix * buf_0_0.u_light_view_projection_matrix;

    vulkan_convert();
}
)";
            String fs = R"(
precision highp float;

UniformTexture(0, 1) uniform sampler2D u_texture;
UniformTexture(0, 3) uniform highp sampler2D u_shadow_texture;

UniformBuffer(0, 2) uniform UniformBuffer02
{
    vec4 u_light_color;
    vec4 u_light_dir;
    float u_light_intensity;
    float u_shadow_strength;
    float u_shadow_z_bias;
    float u_shadow_uv_step;
} buf_0_2;

Input(0) vec2 v_uv;
Input(1) vec3 v_normal;
Input(2) vec4 v_pos_light_proj;

Output(0) vec4 o_frag;

float linear_filter(float z, vec2 uv)
{
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
    {
        return 0.0;
    }
    else
    {
        return z - buf_0_2.u_shadow_z_bias > texture(u_shadow_texture, uv).r ? 1.0 : 0.0;
    }
}

float pcf_filter(float z, vec2 uv)
{
    float shadow = 0.0;
    for (int i = -2; i <= 2; ++i)
    {
        for (int j = -2; j <= 2; ++j)
        {
            vec2 offset = vec2(i, j) * buf_0_2.u_shadow_uv_step;
            shadow += linear_filter(z, uv + offset);
        }
    }
    return shadow / 25.0;
}

float sample_shadow(vec4 pos_light_proj)
{
    vec2 uv = pos_light_proj.xy * 0.5 + 0.5;
    uv.y = 1.0 - uv.y;
    float z = pos_light_proj.z * 0.5 + 0.5;

    return pcf_filter(z, uv) * buf_0_2.u_shadow_strength;
}

void main()
{
    vec4 c = texture(u_texture, v_uv);
    vec3 n = normalize(v_normal);
    vec3 l = normalize(-buf_0_2.u_light_dir.xyz);
    
    float nl = max(dot(n, l), 0.0);
    vec3 diff = c.rgb * nl * buf_0_2.u_light_color.rgb * buf_0_2.u_light_intensity;
    float shadow = sample_shadow(v_pos_light_proj / v_pos_light_proj.w);

    c.rgb = diff * (1.0 - shadow);
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

            for (int i = 0; i < m_renderers.Size(); ++i)
            {
                auto material = m_renderers[i]->GetMaterial();
                material->SetShader(shader);
                material->SetTexture("u_shadow_texture", m_shadow_texture);
                material->SetMatrix("u_light_view_projection_matrix", m_light_view_projection_matrix);
                material->SetFloat("u_shadow_strength", 1.0f);
                material->SetFloat("u_shadow_z_bias", 0.0001f);
                material->SetFloat("u_shadow_uv_step", 1.0f / m_shadow_texture->GetWidth());
            }
        }

        virtual void Init()
        {
            DemoMesh::Init();

            this->InitShadowCaster();
            this->InitShadowReciever();
        }

        virtual void Done()
        {
            m_shadow_texture.reset();
            m_shadow_renderers.Clear();

            Display::Instance()->DestroyCamera(m_shadow_camera);
            m_shadow_camera = nullptr;

            DemoMesh::Done();
        }

        virtual void Update()
        {
            DemoMesh::Update();
        }

        virtual void OnResize(int width, int height)
        {
            DemoMesh::OnResize(width, height);
        }
    };
}
