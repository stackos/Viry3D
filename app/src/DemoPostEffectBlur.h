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
#include "ui/Slider.h"

namespace Viry3D
{
    class DemoPostEffectBlur : public DemoMesh
    {
    public:
        Vector<Camera*> m_blit_cameras;

        void InitPostEffectBlur()
        {
            auto color_texture = Texture::CreateRenderTexture(
                Display::Instance()->GetWidth(),
                Display::Instance()->GetHeight(),
                TextureFormat::R8G8B8A8,
                true,
                FilterMode::Linear,
                SamplerAddressMode::ClampToEdge);
            auto depth_texture = Texture::CreateRenderTexture(
                Display::Instance()->GetWidth(),
                Display::Instance()->GetHeight(),
                Texture::ChooseDepthFormatSupported(true),
                true,
                FilterMode::Linear,
                SamplerAddressMode::ClampToEdge);
            m_camera->SetRenderTarget(color_texture, depth_texture);

            // blur
            int downsample = 1;
            float texel_offset = 1.6f;
            int iter_count = 3;
            float iter_step = 0.0f;

            int width = color_texture->GetWidth();
            int height = color_texture->GetHeight();
            for (int i = 0; i < downsample; ++i)
            {
                width >>= 1;
                height >>= 1;
            }

            auto color_texture_2 = Texture::CreateRenderTexture(
                width,
                height,
                TextureFormat::R8G8B8A8,
                true,
                FilterMode::Linear,
                SamplerAddressMode::ClampToEdge);
            auto color_texture_3 = Texture::CreateRenderTexture(
                width,
                height,
                TextureFormat::R8G8B8A8,
                true,
                FilterMode::Linear,
                SamplerAddressMode::ClampToEdge);

#if VR_VULKAN
            String vs = R"(
Input(0) vec4 a_pos;
Input(2) vec2 a_uv;

Output(0) vec2 v_uv;

void main()
{
	gl_Position = a_pos;
	v_uv = a_uv;

	vulkan_convert();
}
)";
            String fs = R"(
precision highp float;
      
UniformTexture(0, 0) uniform sampler2D u_texture;

UniformBuffer(0, 1) uniform UniformBuffer01
{
	vec4 u_texel_size;
} buf_0_1;

Input(0) vec2 v_uv;

Output(0) vec4 o_frag;

const float kernel[7] = float[7]( 0.0205, 0.0855, 0.232, 0.324, 0.232, 0.0855, 0.0205 );

void main()
{
    vec4 c = vec4(0.0);
    for (int i = 0; i < 7; ++i)
    {
        c += texture(u_texture, v_uv + buf_0_1.u_texel_size.xy * float(i - 3)) * kernel[i];
    }
    o_frag = c;
}
)";
#elif VR_GLES
            String vs = R"(
attribute vec4 a_pos;
attribute vec2 a_uv;

varying vec2 v_uv;

void main()
{
	gl_Position = a_pos;
	v_uv = a_uv;
}
)";
            String fs = R"(
precision highp float;
      
uniform sampler2D u_texture;

uniform vec4 u_texel_size;

varying vec2 v_uv;
            
void main()
{
    float kernel[7];
    kernel[0] = 0.0205;
    kernel[1] = 0.0855;
    kernel[2] = 0.232;
    kernel[3] = 0.324;
    kernel[4] = 0.232;
    kernel[5] = 0.0855;
    kernel[6] = 0.0205;
    
    vec4 c = vec4(0.0);
    for (int i = 0; i < 7; ++i)
    {
        c += texture2D(u_texture, v_uv + u_texel_size.xy * float(i - 3)) * kernel[i];
    }
    gl_FragColor = c;
}
)";
#endif
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

            int camera_depth = 1;

            // color -> color2, down sample
            auto blit_color_camera = Display::Instance()->CreateBlitCamera(camera_depth++, color_texture);
            blit_color_camera->SetRenderTarget(color_texture_2, Ref<Texture>());
            m_blit_cameras.Add(blit_color_camera);

            for (int i = 0; i < iter_count; ++i)
            {
                // color2 -> color, h blur
                auto material_h = RefMake<Material>(shader);
                material_h->SetVector("u_texel_size", Vector4(1.0f / width * texel_offset * (1.0f + i * iter_step), 0, 0, 0));

                blit_color_camera = Display::Instance()->CreateBlitCamera(camera_depth++, color_texture_2, material_h);
                blit_color_camera->SetRenderTarget(color_texture, Ref<Texture>());
                m_blit_cameras.Add(blit_color_camera);

                // color -> color2, v blur
                auto material_v = RefMake<Material>(shader);
                material_v->SetVector("u_texel_size", Vector4(0, 1.0f / height * texel_offset * (1.0f + i * iter_step), 0, 0));

                blit_color_camera = Display::Instance()->CreateBlitCamera(camera_depth++, color_texture, material_v);
                blit_color_camera->SetRenderTarget(color_texture_2, Ref<Texture>());
                m_blit_cameras.Add(blit_color_camera);
            }

            // color -> window
            blit_color_camera = Display::Instance()->CreateBlitCamera(camera_depth++, color_texture_2);
            m_blit_cameras.Add(blit_color_camera);

            m_ui_camera->SetDepth(camera_depth++);
        }

        void InitUI()
        {
            auto canvas = RefMake<CanvasRenderer>();
            m_ui_camera->AddRenderer(canvas);

            // Downsample
            auto label = RefMake<Label>();
            canvas->AddView(label);

            label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetPivot(Vector2(0, 0.5f));
            label->SetSize(Vector2i(100, 30));
            label->SetOffset(Vector2i(40, 120));
            label->SetFont(Font::GetFont(FontType::Consola));
            label->SetFontSize(28);
            label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetText("Downsample");

            auto slider = RefMake<Slider>();
            canvas->AddView(slider);

            slider->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            slider->SetPivot(Vector2(0, 0.5f));
            slider->SetSize(Vector2i(200, 30));
            slider->SetOffset(Vector2i(280, 120));

            // TexelOffset
            label = RefMake<Label>();
            canvas->AddView(label);

            label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetPivot(Vector2(0, 0.5f));
            label->SetSize(Vector2i(100, 30));
            label->SetOffset(Vector2i(40, 185));
            label->SetFont(Font::GetFont(FontType::Consola));
            label->SetFontSize(28);
            label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetText("TexelOffset");

            slider = RefMake<Slider>();
            canvas->AddView(slider);

            slider->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            slider->SetPivot(Vector2(0, 0.5f));
            slider->SetSize(Vector2i(200, 30));
            slider->SetOffset(Vector2i(280, 185));

            // IterCount
            label = RefMake<Label>();
            canvas->AddView(label);

            label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetPivot(Vector2(0, 0.5f));
            label->SetSize(Vector2i(100, 30));
            label->SetOffset(Vector2i(40, 250));
            label->SetFont(Font::GetFont(FontType::Consola));
            label->SetFontSize(28);
            label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetText("IterCount");

            slider = RefMake<Slider>();
            canvas->AddView(slider);

            slider->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            slider->SetPivot(Vector2(0, 0.5f));
            slider->SetSize(Vector2i(200, 30));
            slider->SetOffset(Vector2i(280, 250));

            // IterStep
            label = RefMake<Label>();
            canvas->AddView(label);

            label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetPivot(Vector2(0, 0.5f));
            label->SetSize(Vector2i(100, 30));
            label->SetOffset(Vector2i(40, 315));
            label->SetFont(Font::GetFont(FontType::Consola));
            label->SetFontSize(28);
            label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetText("IterStep");

            slider = RefMake<Slider>();
            canvas->AddView(slider);

            slider->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            slider->SetPivot(Vector2(0, 0.5f));
            slider->SetSize(Vector2i(200, 30));
            slider->SetOffset(Vector2i(280, 315));
        }

        virtual void Init()
        {
            DemoMesh::Init();

            this->InitPostEffectBlur();
            this->InitUI();
        }

        virtual void Done()
        {
            for (auto i : m_blit_cameras)
            {
                Display::Instance()->DestroyCamera(i);
            }
            m_blit_cameras.Clear();

            DemoMesh::Done();
        }

        virtual void Update()
        {
            DemoMesh::Update();
        }
    };
}
