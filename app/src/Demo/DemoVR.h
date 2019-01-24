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

#include "DemoLightmap.h"

namespace Viry3D
{
    class DemoVR : public DemoLightmap
    {
    public:
        Camera* m_blit_color_camera = nullptr;

        void InitShader()
        {
            RenderState render_state;
            auto shader = RefMake<Shader>(
                "#define LIGHTMAP 1\n"
                "#define MULTIVIEW 1",
                Vector<String>({ "Diffuse.vs" }),
                "",
                "#define LIGHTMAP 1",
                Vector<String>({ "Diffuse.fs" }),
                "",
                render_state);
            Shader::AddCache("Diffuse", shader);
        }

        void InitVR()
        {
            m_camera->SetStereoRendering(true);

            auto color_texture = Texture::CreateRenderTexture(
                Display::Instance()->GetWidth() / 2,
                Display::Instance()->GetHeight(),
                TextureFormat::R8G8B8A8,
                2,
                1,
                true,
                FilterMode::Nearest,
                SamplerAddressMode::ClampToEdge);
            auto depth_texture = Texture::CreateRenderTexture(
                Display::Instance()->GetWidth() / 2,
                Display::Instance()->GetHeight(),
                Texture::ChooseDepthFormatSupported(true),
                2,
                1,
                true,
                FilterMode::Nearest,
                SamplerAddressMode::ClampToEdge);
            m_camera->SetRenderTarget(color_texture, depth_texture);

            // color -> window
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
      
UniformTexture(0, 0) uniform lowp sampler2DArray u_texture;

Input(0) vec2 v_uv;

Output(0) vec4 o_frag;

void main()
{
    if (v_uv.x < 0.5)
    {
        o_frag = texture(u_texture, vec3(v_uv.x * 2.0, v_uv.y, 0.0));
    }
    else
    {
        o_frag = texture(u_texture, vec3((v_uv.x - 0.5) * 2.0, v_uv.y, 1.0));
    }
}
)";
            RenderState render_state;
            render_state.cull = RenderState::Cull::Off;
            render_state.zTest = RenderState::ZTest::Off;
            render_state.zWrite = RenderState::ZWrite::Off;

            auto shader = RefMake<Shader>(
                "",
                Vector<String>(),
                vs,
                "",
                Vector<String>(),
                fs,
                render_state);
            auto material = RefMake<Material>(shader);
            material->SetTexture("u_texture", color_texture);

            m_blit_color_camera = Display::Instance()->CreateBlitCamera(1, material);

            m_ui_camera->SetDepth(2);
        }

        virtual void Init()
        {
            this->InitCamera();
            this->InitLight();
            this->InitShader();
            this->InitScene();
            this->InitUI();
            this->InitVR();
        }

        virtual void Done()
        {
            Display::Instance()->DestroyCamera(m_blit_color_camera);
            m_blit_color_camera = nullptr;

            DemoLightmap::Done();
        }

        virtual void Update()
        {
            DemoLightmap::Update();
        }
    };
}
