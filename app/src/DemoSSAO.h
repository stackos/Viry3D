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

#include "Demo.h"
#include "Application.h"
#include "graphics/Display.h"
#include "graphics/Camera.h"
#include "graphics/Shader.h"
#include "graphics/Material.h"
#include "graphics/MeshRenderer.h"
#include "graphics/Mesh.h"
#include "graphics/Texture.h"
#include "graphics/Light.h"
#include "math/Quaternion.h"
#include "time/Time.h"
#include "ui/CanvasRenderer.h"
#include "ui/Label.h"
#include "ui/Font.h"
#include "Resources.h"

#define KERNEL_SIZE 32
#define NOISE_SIZE 4
#define RADIUS 0.5f

namespace Viry3D
{
    class DemoSSAO : public Demo
    {
    public:
        struct CameraParam
        {
            Vector3 pos;
            Vector3 rot;
            float fov;
            float near_clip;
            float far_clip;
        };
        CameraParam m_camera_param = {
            Vector3(0, 5, -10),
            Vector3(30, 0, 0),
            45,
            0.3f,
            1000
        };
        struct LightParam
        {
            Color ambient_color;
            Color light_color;
            float light_intensity;
        };
        LightParam m_light_param = {
            Color(0.2f, 0.2f, 0.2f, 1),
            Color(1, 1, 1, 1),
            0.8f
        };

        Camera* m_camera = nullptr;
        Camera* m_ui_camera = nullptr;
        Label* m_label = nullptr;
        Ref<Light> m_light;
        Camera* m_ssao_camera = nullptr;
        Camera* m_blur_camera = nullptr;
        Camera* m_blit_color_camera = nullptr;

        void InitRenderTexture()
        {
            auto color_texture = Texture::CreateRenderTexture(
                Display::Instance()->GetWidth(),
                Display::Instance()->GetHeight(),
                TextureFormat::R8G8B8A8,
                1,
                1,
                true,
                FilterMode::Nearest,
                SamplerAddressMode::ClampToEdge);
            auto depth_texture = Texture::CreateRenderTexture(
                Display::Instance()->GetWidth(),
                Display::Instance()->GetHeight(),
                Texture::ChooseDepthFormatSupported(true),
                1,
                1,
                true,
                FilterMode::Nearest,
                SamplerAddressMode::ClampToEdge);
            m_camera->SetRenderTarget(color_texture, depth_texture);

            auto pos_texture = Texture::CreateRenderTexture(
                Display::Instance()->GetWidth(),
                Display::Instance()->GetHeight(),
                TextureFormat::R16G16B16A16F,
                1,
                1,
                true,
                FilterMode::Nearest,
                SamplerAddressMode::ClampToEdge);
            auto normal_texture = Texture::CreateRenderTexture(
                Display::Instance()->GetWidth(),
                Display::Instance()->GetHeight(),
                TextureFormat::R8G8B8A8,
                1,
                1,
                true,
                FilterMode::Nearest,
                SamplerAddressMode::ClampToEdge);
            m_camera->SetExtraRenderTargets({ pos_texture, normal_texture });

            // ssao param
            Vector<Vector4> noise(NOISE_SIZE * NOISE_SIZE);
            for (int i = 0; i < noise.Size(); ++i)
            {
                float x = Mathf::RandomRange(-1.0f, 1.0f);
                float y = Mathf::RandomRange(-1.0f, 1.0f);
                noise[i] = Vector4(x, y, 0, 0);
            }
            auto noise_texture = Texture::CreateTexture2DFromMemory(
                ByteBuffer((byte*) &noise[0], noise.SizeInBytes()),
                NOISE_SIZE, NOISE_SIZE,
                TextureFormat::R16G16B16A16F,
                FilterMode::Nearest,
                SamplerAddressMode::Repeat,
                false,
                false,
                false);

            Vector<Vector4> kernel(KERNEL_SIZE);
            for (int i = 0; i < kernel.Size(); i++)
            {
                float x = Mathf::RandomRange(-1.0f, 1.0f);
                float y = Mathf::RandomRange(-1.0f, 1.0f);
                float z = Mathf::RandomRange(0.0f, 1.0f);
                Vector3 sample = Vector3::Normalize(Vector3(x, y, z));
                sample *= Mathf::RandomRange(0.0f, 1.0f);

                float scale = i / (float) KERNEL_SIZE;
                scale = Mathf::Lerp(0.1f, 1.0f, scale * scale);
                sample *= scale;

                kernel[i] = Vector4(sample);
            }

            // ssao shader
            RenderState render_state;
            render_state.cull = RenderState::Cull::Off;
            render_state.zTest = RenderState::ZTest::Off;
            render_state.zWrite = RenderState::ZWrite::Off;

            String vs = R"(
Input(0) vec3 a_pos;
Input(2) vec2 a_uv;

Output(0) vec2 v_uv;

void main()
{
	gl_Position = vec4(a_pos, 1.0);
	v_uv = a_uv;

	vulkan_convert();
}
)";
            String fs = R"(
precision highp float;

UniformBuffer(0, 0) uniform UniformBuffer00
{
    vec4 u_noise_scale;
    vec4 u_kernel[KERNEL_SIZE];
    mat4 u_projection;
} buf_0_0;

UniformTexture(0, 1) uniform sampler2D u_pos_texture;
UniformTexture(0, 2) uniform sampler2D u_normal_texture;
UniformTexture(0, 3) uniform sampler2D u_noise_texture;

Input(0) vec2 v_uv;

Output(0) float o_frag;

void main()
{
    vec3 pos = texture(u_pos_texture, v_uv).xyz;
    vec3 normal = texture(u_normal_texture, v_uv).xyz * 2.0 - 1.0;
    vec3 random = texture(u_noise_texture, v_uv * buf_0_0.u_noise_scale.xy).xyz;
    vec3 tangent = normalize(random - normal * dot(random, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 tbn = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for(int i = 0; i < KERNEL_SIZE; ++i)
    {
        vec3 sample_pos = tbn * buf_0_0.u_kernel[i].xyz;
        sample_pos = pos + sample_pos * RADIUS;
        
        vec4 offset = vec4(sample_pos, 1.0);
        offset = offset * buf_0_0.u_projection;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        float depth = -texture(u_pos_texture, vec2(offset.x, 1.0 - offset.y)).w;
        
        float check = smoothstep(0.0, 1.0, RADIUS / abs(pos.z - depth));
        occlusion += (depth >= sample_pos.z ? 1.0 : 0.0) * check;
    }
    occlusion = 1.0 - (occlusion / float(KERNEL_SIZE));

    o_frag = occlusion;
}
)";

            auto ssao_shader = RefMake<Shader>(
                "",
                Vector<String>(),
                vs,
                String::Format("#define KERNEL_SIZE %d\n#define RADIUS %f", KERNEL_SIZE, RADIUS),
                Vector<String>(),
                fs,
                render_state);
            auto ssao_material = RefMake<Material>(ssao_shader);
            ssao_material->SetTexture("u_pos_texture", pos_texture);
            ssao_material->SetTexture("u_normal_texture", normal_texture);
            ssao_material->SetTexture("u_noise_texture", noise_texture);
            ssao_material->SetVector("u_noise_scale", Vector4(Display::Instance()->GetWidth() / (float) NOISE_SIZE, Display::Instance()->GetHeight() / (float) NOISE_SIZE, 0, 0));
            ssao_material->SetVectorArray("u_kernel", kernel);
            ssao_material->SetMatrix("u_projection", m_camera->GetProjectionMatrix());

            auto ssao_texture = Texture::CreateRenderTexture(
                Display::Instance()->GetWidth(),
                Display::Instance()->GetHeight(),
                TextureFormat::R8,
                1,
                1,
                true,
                FilterMode::Nearest,
                SamplerAddressMode::ClampToEdge);

            m_ssao_camera = Display::Instance()->CreateBlitCamera(1, ssao_material);
            m_ssao_camera->SetRenderTarget(ssao_texture, Ref<Texture>());

            // ssao blur
            fs = R"(
precision highp float;

UniformTexture(0, 0) uniform sampler2D u_ssao_texture;

Input(0) vec2 v_uv;

Output(0) float o_frag;

void main()
{
    float result = 0.0;

    vec2 texel_size = 1.0 / vec2(textureSize(u_ssao_texture, 0));
    int half_noise_size = NOISE_SIZE / 2;
    for (int i = -half_noise_size; i < half_noise_size; ++i) 
    {
        for (int j = -half_noise_size; j < half_noise_size; ++j) 
        {
            vec2 offset = vec2(float(i), float(j)) * texel_size;
            result += texture(u_ssao_texture, v_uv + offset).r;
        }
    }
    result /= float(NOISE_SIZE * NOISE_SIZE);

    o_frag = result;
}
)";

            auto blur_shader = RefMake<Shader>(
                "",
                Vector<String>(),
                vs,
                String::Format("#define NOISE_SIZE %d", NOISE_SIZE),
                Vector<String>(),
                fs,
                render_state);
            auto blur_material = RefMake<Material>(blur_shader);
            blur_material->SetTexture("u_ssao_texture", ssao_texture);

            auto blur_texture = Texture::CreateRenderTexture(
                Display::Instance()->GetWidth(),
                Display::Instance()->GetHeight(),
                TextureFormat::R8,
                1,
                1,
                true,
                FilterMode::Nearest,
                SamplerAddressMode::ClampToEdge);

            m_blur_camera = Display::Instance()->CreateBlitCamera(2, blur_material);
            m_blur_camera->SetRenderTarget(blur_texture, Ref<Texture>());

            // composite
            vs = R"(
UniformBuffer(0, 0) uniform UniformBuffer00
{
    mat4 u_view;
    vec4 u_light_pos;
} buf_0_0;

Input(0) vec3 a_pos;
Input(2) vec2 a_uv;

Output(0) vec2 v_uv;
Output(1) vec3 v_light_dir;

void main()
{
	gl_Position = vec4(a_pos, 1.0);
	v_uv = a_uv;
    
    vec3 light_dir = normalize(-buf_0_0.u_light_pos.xyz); // directional light
    v_light_dir = (vec4(light_dir, 0.0) * buf_0_0.u_view).xyz;

	vulkan_convert();
}
)";

            fs = R"(
precision highp float;

UniformBuffer(0, 1) uniform UniformBuffer01
{
    vec4 u_ambient_color;
	vec4 u_light_color;
	float u_light_intensity;
} buf_0_1;

UniformTexture(0, 2) uniform sampler2D u_color_texture;
UniformTexture(0, 3) uniform sampler2D u_ssao_texture;
UniformTexture(0, 4) uniform sampler2D u_normal_texture;

Input(0) vec2 v_uv;
Input(1) vec3 v_light_dir;

Output(0) vec4 o_frag;

void main()
{
    vec4 c = texture(u_color_texture, v_uv);
    vec3 n = texture(u_normal_texture, v_uv).xyz * 2.0 - 1.0;
    vec3 l = normalize(v_light_dir);

    float nl = max(dot(n, l), 0.0);
    vec3 gi_diffuse = c.rgb * buf_0_1.u_ambient_color.rgb;
    vec3 diffuse = c.rgb * nl * buf_0_1.u_light_color.rgb * buf_0_1.u_light_intensity;

    c.rgb = gi_diffuse + diffuse;
    c.a = 1.0;

    float occlusion = texture(u_ssao_texture, v_uv).r;
    c.rgb *= occlusion;

    o_frag = c;
}
)";

            auto composite_shader = RefMake<Shader>(
                "",
                Vector<String>(),
                vs,
                "",
                Vector<String>(),
                fs,
                render_state);
            auto composite_material = RefMake<Material>(composite_shader);
            composite_material->SetTexture("u_color_texture", color_texture);
            composite_material->SetTexture("u_ssao_texture", blur_texture);
            composite_material->SetTexture("u_normal_texture", normal_texture);
            composite_material->SetMatrix("u_view", m_camera->GetViewMatrix());
            composite_material->SetLightProperties(m_light);

            m_blit_color_camera = Display::Instance()->CreateBlitCamera(3, composite_material);

            m_ui_camera->SetDepth(4);
        }

        void InitCamera()
        {
            m_camera = Display::Instance()->CreateCamera();
            m_camera->SetDepth(0);
            m_camera->SetLocalPosition(m_camera_param.pos);
            m_camera->SetLocalRotation(Quaternion::Euler(m_camera_param.rot));

            m_camera->SetFieldOfView(m_camera_param.fov);
            m_camera->SetNearClip(m_camera_param.near_clip);
            m_camera->SetFarClip(m_camera_param.far_clip);
        }

        void InitLight()
        {
            m_light = RefMake<Light>(LightType::Directional);
            m_light->SetLocalRotation(Quaternion::Euler(45, 60, 0));
            m_light->SetAmbientColor(m_light_param.ambient_color);
            m_light->SetColor(m_light_param.light_color);
            m_light->SetIntensity(m_light_param.light_intensity);
        }

        void InitScene()
        {
            RenderState render_state;
            auto shader = RefMake<Shader>(
                "",
                Vector<String>({ "DeferredGeometry.vs" }),
                "",
                "",
                Vector<String>({ "DeferredGeometry.fs" }),
                "",
                render_state);
            Shader::AddCache("Diffuse", shader);

            auto node = Resources::LoadNode("res/scene/lightmap/scene.go");
            for (int i = 0; i < node->GetChildCount(); ++i)
            {
                Ref<MeshRenderer> mesh_renderer = RefCast<MeshRenderer>(node->GetChild(i));
                if (mesh_renderer)
                {
                    m_camera->AddRenderer(mesh_renderer);

                    auto material = mesh_renderer->GetMaterial();
                    if (material)
                    {
                        if (!material->GetTexture("u_texture"))
                        {
                            material->SetTexture("u_texture", Texture::GetSharedWhiteTexture());
                        }
                        material->SetLightProperties(m_light);
                        material->SetVector("u_clip_plane", Vector4(m_camera_param.near_clip, m_camera_param.far_clip, 0, 0));
                    }
                }
            }
        }

        void InitUI()
        {
            m_ui_camera = Display::Instance()->CreateCamera();
            m_ui_camera->SetDepth(1);
            m_ui_camera->SetClearFlags(CameraClearFlags::Nothing);

            auto canvas = RefMake<CanvasRenderer>();
            m_ui_camera->AddRenderer(canvas);

            auto label = RefMake<Label>();
            canvas->AddView(label);

            label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetPivot(Vector2(0, 0));
            label->SetSize(Vector2i(100, 30));
            label->SetOffset(Vector2i(40, 40));
            label->SetFont(Font::GetFont(FontType::Consola));
            label->SetFontSize(28);
            label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);

            m_label = label.get();
        }

        virtual void Init()
        {
            this->InitCamera();
            this->InitLight();
            this->InitScene();
            this->InitUI();
            this->InitRenderTexture();
        }

        virtual void Done()
        {
            if (m_ssao_camera)
            {
                Display::Instance()->DestroyCamera(m_ssao_camera);
                m_ssao_camera = nullptr;
            }
            if (m_blur_camera)
            {
                Display::Instance()->DestroyCamera(m_blur_camera);
                m_blur_camera = nullptr;
            }
            if (m_blit_color_camera)
            {
                Display::Instance()->DestroyCamera(m_blit_color_camera);
                m_blit_color_camera = nullptr;
            }
            if (m_ui_camera)
            {
                Display::Instance()->DestroyCamera(m_ui_camera);
                m_ui_camera = nullptr;
            }
            if (m_camera)
            {
                Display::Instance()->DestroyCamera(m_camera);
                m_camera = nullptr;
            }

            Shader::RemoveCache("Diffuse");
        }

        virtual void Update()
        {
            if (m_label)
            {
                m_label->SetText(String::Format("FPS:%d", Time::GetFPS()));
            }
        }
    };
}
