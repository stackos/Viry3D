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
#include "graphics/Light.h"
#include "math/Quaternion.h"
#include "time/Time.h"
#include "ui/CanvasRenderer.h"
#include "ui/Label.h"
#include "ui/Font.h"
#include "Resources.h"

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
        Camera* m_blit_color_camera = nullptr;

        void InitRenderTexture()
        {
            auto color_texture = Texture::CreateRenderTexture(
                Display::Instance()->GetWidth(),
                Display::Instance()->GetHeight(),
                TextureFormat::R8G8B8A8,
                1,
                true,
                FilterMode::Nearest,
                SamplerAddressMode::ClampToEdge);
            auto depth_texture = Texture::CreateRenderTexture(
                Display::Instance()->GetWidth(),
                Display::Instance()->GetHeight(),
                Texture::ChooseDepthFormatSupported(true),
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
                true,
                FilterMode::Nearest,
                SamplerAddressMode::ClampToEdge);
            auto normal_texture = Texture::CreateRenderTexture(
                Display::Instance()->GetWidth(),
                Display::Instance()->GetHeight(),
                TextureFormat::R16G16B16A16F,
                1,
                true,
                FilterMode::Nearest,
                SamplerAddressMode::ClampToEdge);
            m_camera->SetExtraRenderTargets({ pos_texture, normal_texture });

            Vector<Vector4> noise;
            for (int i = 0; i < 16; ++i)
            {
                float x = Mathf::RandomRange(-1.0f, 1.0f);
                float y = Mathf::RandomRange(-1.0f, 1.0f);
                noise.Add(Vector4(x, y, 0, 0));
            }
            auto noise_texture = Texture::CreateTexture2DFromMemory(
                ByteBuffer((byte*) &noise[0], noise.SizeInBytes()),
                4, 4,
                TextureFormat::R16G16B16A16F,
                FilterMode::Nearest,
                SamplerAddressMode::Repeat,
                false,
                false);

            Vector<Vector4> kernel;
            for (int i = 0; i < 64; i++)
            {
                float x = Mathf::RandomRange(-1.0f, 1.0f);
                float y = Mathf::RandomRange(-1.0f, 1.0f);
                float z = Mathf::RandomRange(0.0f, 1.0f);
                Vector3 sample = Vector3::Normalize(Vector3(x, y, z));
                sample *= Mathf::RandomRange(0.0f, 1.0f);

                float scale = i / 64.0f;
                scale = Mathf::Lerp(0.1f, 1.0f, scale * scale);
                sample *= scale;

                kernel.Add(Vector4(sample));
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

#define KERNEL_SIZE 64

UniformBuffer(0, 0) uniform UniformBuffer01
{
    vec4 u_noise_scale;
    vec4 u_kernel[KERNEL_SIZE];
    mat4 u_projection;
} buf_0_1;

UniformTexture(0, 1) uniform sampler2D u_pos_texture;
UniformTexture(0, 2) uniform sampler2D u_normal_texture;
UniformTexture(0, 3) uniform sampler2D u_noise_texture;

Input(0) vec2 v_uv;

Output(0) vec4 o_frag;

void main()
{
    vec3 pos = texture(u_pos_texture, v_uv).xyz;
    vec3 normal = texture(u_normal_texture, v_uv).xyz;
    vec3 random = texture(u_noise_texture, v_uv * buf_0_1.u_noise_scale.xy).xyz;
    vec3 tangent = normalize(random - normal * dot(random, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 tbn = mat3(tangent, bitangent, normal);

    const float radius = 1.0;

    float occlusion = 0.0;
    for(int i = 0; i < KERNEL_SIZE; ++i)
    {
        // get sample position
        vec3 sample_pos = tbn * buf_0_1.u_kernel[i].xyz; // from tangent to view-space
        sample_pos = pos + sample_pos * radius;
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(sample_pos, 1.0);
        offset = offset * buf_0_1.u_projection; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        float depth = texture(u_pos_texture, vec2(offset.x, 1.0 - offset.y)).z; // get depth value of kernel sample
        
        // range check & accumulate
        float check = smoothstep(0.0, 1.0, radius / abs(pos.z - depth));
        occlusion += (depth >= sample_pos.z ? 1.0 : 0.0) * check;
    }
    occlusion = 1.0 - (occlusion / float(KERNEL_SIZE));

    o_frag = vec4(occlusion);
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
            auto material = RefMake<Material>(shader);
            material->SetTexture("u_pos_texture", pos_texture);
            material->SetTexture("u_normal_texture", normal_texture);
            material->SetTexture("u_noise_texture", noise_texture);
            material->SetVector("u_noise_scale", Vector4(Display::Instance()->GetWidth() / (float) 4, Display::Instance()->GetHeight() / (float) 4, 0, 0));
            material->SetVectorArray("u_kernel", kernel);
            material->SetMatrix("u_projection", m_camera->GetProjectionMatrix());

            // color -> window
            m_blit_color_camera = Display::Instance()->CreateBlitCamera(1, Ref<Texture>(), material);

            m_ui_camera->SetDepth(2);
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
                Vector<String>({ "DeferredGeometry.vs.in" }),
                "",
                "",
                Vector<String>({ "DeferredGeometry.fs.in" }),
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
