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

#define SHADOW_MAP_SIZE 1024

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

            m_shadow_texture = Texture::CreateRenderTexture(
                SHADOW_MAP_SIZE,
                SHADOW_MAP_SIZE,
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

            int target_width = SHADOW_MAP_SIZE;
            int target_height = SHADOW_MAP_SIZE;
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
                renderer->SetLocalPosition(m_renderers[i]->GetLocalPosition());
                renderer->SetLocalRotation(m_renderers[i]->GetLocalRotation());
                renderer->SetLocalScale(m_renderers[i]->GetLocalScale());

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
    float u_shadow_filter_radius;
} buf_0_2;

Input(0) vec2 v_uv;
Input(1) vec3 v_normal;
Input(2) vec4 v_pos_light_proj;

Output(0) vec4 o_frag;

const vec2 Poisson25[25] = vec2[](
    vec2(-0.978698, -0.0884121),
    vec2(-0.841121, 0.521165),
    vec2(-0.71746, -0.50322),
    vec2(-0.702933, 0.903134),
    vec2(-0.663198, 0.15482),
    vec2(-0.495102, -0.232887),
    vec2(-0.364238, -0.961791),
    vec2(-0.345866, -0.564379),
    vec2(-0.325663, 0.64037),
    vec2(-0.182714, 0.321329),
    vec2(-0.142613, -0.0227363),
    vec2(-0.0564287, -0.36729),
    vec2(-0.0185858, 0.918882),
    vec2(0.0381787, -0.728996),
    vec2(0.16599, 0.093112),
    vec2(0.253639, 0.719535),
    vec2(0.369549, -0.655019),
    vec2(0.423627, 0.429975),
    vec2(0.530747, -0.364971),
    vec2(0.566027, -0.940489),
    vec2(0.639332, 0.0284127),
    vec2(0.652089, 0.669668),
    vec2(0.773797, 0.345012),
    vec2(0.968871, 0.840449),
    vec2(0.991882, -0.657338)
);

float bias_z(float z, vec2 dz_duv, vec2 offset)
{
    return z + dot(dz_duv, offset);
}

float linear_filter(vec2 uv)
{
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
    {
        return 1.0;
    }
    else
    {
        return texture(u_shadow_texture, uv).r;
    }
}

float poisson_filter(float z, vec2 uv, vec2 dz_duv, vec2 filter_radius)
{
    float shadow = 0.0;
    for (int i = 0; i < 25; ++i)
    {
        vec2 offset = Poisson25[i] * filter_radius;
        float shadow_depth = linear_filter(uv + offset);
        if (bias_z(z, dz_duv, offset) - buf_0_2.u_shadow_z_bias > shadow_depth)
        {
            shadow += 1.0;
        }
    }
    return shadow / 25.0;
}

vec2 depth_gradient(vec2 uv, float z)
{
    vec2 dz_duv = vec2(0.0, 0.0);

    vec3 duvdist_dx = dFdx(vec3(uv,z));
    vec3 duvdist_dy = dFdy(vec3(uv,z));

    dz_duv.x = duvdist_dy.y * duvdist_dx.z;
    dz_duv.x -= duvdist_dx.y * duvdist_dy.z;

    dz_duv.y = duvdist_dx.x * duvdist_dy.z;
    dz_duv.y -= duvdist_dy.x * duvdist_dx.z;

    float det = (duvdist_dx.x * duvdist_dy.y) - (duvdist_dx.y * duvdist_dy.x);
    dz_duv /= det;

    return dz_duv;
}

float sample_shadow(vec4 pos_light_proj)
{
    vec2 uv = pos_light_proj.xy * 0.5 + 0.5;
    uv.y = 1.0 - uv.y;
    float z = pos_light_proj.z * 0.5 + 0.5;
    vec2 filter_radius = vec2(buf_0_2.u_shadow_filter_radius);
    vec2 dz_duv = depth_gradient(uv, z);

    return poisson_filter(z, uv, dz_duv, filter_radius);
}

void main()
{
    vec4 c = texture(u_texture, v_uv);
    vec3 n = normalize(v_normal);
    vec3 l = normalize(-buf_0_2.u_light_dir.xyz);
    
    float nl = max(dot(n, l), 0.0);
    vec3 diff = c.rgb * nl * buf_0_2.u_light_color.rgb * buf_0_2.u_light_intensity;
    float shadow = sample_shadow(v_pos_light_proj / v_pos_light_proj.w) * buf_0_2.u_shadow_strength;

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
                material->SetFloat("u_shadow_z_bias", 0.001f);
                material->SetFloat("u_shadow_filter_radius", 1.0f / SHADOW_MAP_SIZE * 3);
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
