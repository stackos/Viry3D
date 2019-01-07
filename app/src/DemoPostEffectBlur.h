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
#include "ui/Slider.h"
#include "Debug.h"

namespace Viry3D
{
    class DemoPostEffectBlur : public DemoMesh
    {
    public:
        Vector<Camera*> m_blit_cameras;
        int m_downsample = 2;
        float m_texel_offset = 1.6f;
        int m_iter_count = 3;
        float m_iter_step = 0.0f;
        int m_downsample_target = m_downsample;
        float m_texel_offset_target = m_texel_offset;
        int m_iter_count_target = m_iter_count;
        float m_iter_step_target = m_iter_step;

        void InitPostEffectBlur()
        {
            int width = Display::Instance()->GetWidth();
            int height = Display::Instance()->GetHeight();
            for (int i = 0; i < m_downsample; ++i)
            {
                width >>= 1;
                height >>= 1;
            }
            if (width == 0 || height == 0)
            {
                return;
            }

            auto color_texture = Texture::CreateRenderTexture(
                Display::Instance()->GetWidth(),
                Display::Instance()->GetHeight(),
                TextureFormat::R8G8B8A8,
                1,
                1,
                true,
                FilterMode::Linear,
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

            // blur
            auto color_texture_2 = Texture::CreateRenderTexture(
                width,
                height,
                TextureFormat::R8G8B8A8,
                1,
                1,
                true,
                FilterMode::Linear,
                SamplerAddressMode::ClampToEdge);
            auto color_texture_3 = Texture::CreateRenderTexture(
                width,
                height,
                TextureFormat::R8G8B8A8,
                1,
                1,
                true,
                FilterMode::Linear,
                SamplerAddressMode::ClampToEdge);

#if VR_VULKAN
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
attribute vec3 a_pos;
attribute vec2 a_uv;

varying vec2 v_uv;

void main()
{
	gl_Position = vec4(a_pos, 1.0);
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

            for (int i = 0; i < m_iter_count; ++i)
            {
                // color2 -> color3, h blur
                auto material_h = RefMake<Material>(shader);
                material_h->SetVector("u_texel_size", Vector4(1.0f / width * m_texel_offset * (1.0f + i * m_iter_step), 0, 0, 0));
                material_h->SetTexture("u_texture", color_texture_2);

                blit_color_camera = Display::Instance()->CreateBlitCamera(camera_depth++, material_h);
                blit_color_camera->SetRenderTarget(color_texture_3, Ref<Texture>());
                m_blit_cameras.Add(blit_color_camera);

                // color3 -> color2, v blur
                auto material_v = RefMake<Material>(shader);
                material_v->SetVector("u_texel_size", Vector4(0, 1.0f / height * m_texel_offset * (1.0f + i * m_iter_step), 0, 0));
                material_v->SetTexture("u_texture", color_texture_3);

                blit_color_camera = Display::Instance()->CreateBlitCamera(camera_depth++, material_v);
                blit_color_camera->SetRenderTarget(color_texture_2, Ref<Texture>());
                m_blit_cameras.Add(blit_color_camera);
            }

            // color2 -> window
            blit_color_camera = Display::Instance()->CreateBlitCamera(camera_depth++, color_texture_2);
            m_blit_cameras.Add(blit_color_camera);

            m_ui_camera->SetDepth(camera_depth++);
        }

        void InitUI()
        {
            auto canvas = RefMake<CanvasRenderer>();
            m_ui_camera->AddRenderer(canvas);

            Vector<String> slider_names({
                "Downsample",
                "TexelOffset",
                "IterCount",
                "IterStep",
                });
            Vector<Ref<Slider>> sliders;
            Vector<Ref<Label>> values;

            for (int i = 0; i < slider_names.Size(); ++i)
            {
                auto label = RefMake<Label>();
                canvas->AddView(label);

                int y = 120 + i * 65;

                label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
                label->SetPivot(Vector2(0, 0.5f));
                label->SetSize(Vector2i(100, 30));
                label->SetOffset(Vector2i(40, y));
                label->SetFont(Font::GetFont(FontType::Consola));
                label->SetFontSize(28);
                label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);
                label->SetText(slider_names[i]);

                auto slider = RefMake<Slider>();
                canvas->AddView(slider);
                sliders.Add(slider);

                slider->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
                slider->SetPivot(Vector2(0, 0.5f));
                slider->SetSize(Vector2i(200, 30));
                slider->SetOffset(Vector2i(280, y));

                label = RefMake<Label>();
                canvas->AddView(label);
                values.Add(label);

                label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
                label->SetPivot(Vector2(0, 0.5f));
                label->SetSize(Vector2i(100, 30));
                label->SetOffset(Vector2i(500, y));
                label->SetFont(Font::GetFont(FontType::Consola));
                label->SetFontSize(28);
                label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);
            }

            sliders[0]->SetProgress((m_downsample - 1) / (float) (10 - 1));
            sliders[0]->SetValueType(Slider::ValueType::Int, Slider::Value(1), Slider::Value(10));
            sliders[0]->SetOnValueChange([=](const Slider::Value& value) {
                m_downsample_target = value.int_value;
                values[0]->SetText(String::Format("%d", m_downsample_target));
            });
            values[0]->SetText(String::Format("%d", m_downsample));

            sliders[1]->SetProgress((m_texel_offset - 0.0f) / (10.0f - 0.0f));
            sliders[1]->SetValueType(Slider::ValueType::Float, Slider::Value(0.0f), Slider::Value(10.0f));
            sliders[1]->SetOnValueChange([=](const Slider::Value& value) {
                m_texel_offset_target = value.float_value;
                values[1]->SetText(String::Format("%.2f", m_texel_offset_target));
            });
            values[1]->SetText(String::Format("%.2f", m_texel_offset));

            sliders[2]->SetProgress((m_iter_count - 1) / (float) (10 - 1));
            sliders[2]->SetValueType(Slider::ValueType::Int, Slider::Value(1), Slider::Value(10));
            sliders[2]->SetOnValueChange([=](const Slider::Value& value) {
                m_iter_count_target = value.int_value;
                values[2]->SetText(String::Format("%d", m_iter_count_target));
            });
            values[2]->SetText(String::Format("%d", m_iter_count));

            sliders[3]->SetProgress((m_iter_step - 0.0f) / (10.0f - 0.0f));
            sliders[3]->SetValueType(Slider::ValueType::Float, Slider::Value(0.0f), Slider::Value(10.0f));
            sliders[3]->SetOnValueChange([=](const Slider::Value& value) {
                m_iter_step_target = value.float_value;
                values[3]->SetText(String::Format("%.2f", m_iter_step_target));
            });
            values[3]->SetText(String::Format("%.2f", m_iter_step));
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

            if (m_downsample != m_downsample_target ||
                m_texel_offset != m_texel_offset_target ||
                m_iter_count != m_iter_count_target ||
                m_iter_step != m_iter_step_target)
            {
                m_downsample = m_downsample_target;
                m_texel_offset = m_texel_offset_target;
                m_iter_count = m_iter_count_target;
                m_iter_step = m_iter_step_target;

                m_camera->SetRenderTarget(Ref<Texture>(), Ref<Texture>());

                for (auto i : m_blit_cameras)
                {
                    Display::Instance()->DestroyCamera(i);
                }
                m_blit_cameras.Clear();

                this->InitPostEffectBlur();
            }
        }
    };
}
