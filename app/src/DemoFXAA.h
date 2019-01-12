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
#include "ui/SwitchButton.h"
#include "ui/SelectButton.h"

#define FXAA_QUALITY_FAST		10
#define FXAA_QUALITY_DEFAULT	12
#define FXAA_QUALITY_HIGH		29
#define FXAA_QUALITY_EXTREME	39

namespace Viry3D
{
    class DemoFXAA : public DemoMesh
    {
    public:
        Camera* m_blit_camera = nullptr;
        int m_target_quality = 1;

        void InitRenderTexture()
        {
            const int qualities[] = {
                FXAA_QUALITY_FAST,
                FXAA_QUALITY_DEFAULT,
                FXAA_QUALITY_HIGH,
                FXAA_QUALITY_EXTREME,
            };
            int quality = qualities[m_target_quality];

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

            RenderState render_state;
            render_state.cull = RenderState::Cull::Off;
            render_state.zTest = RenderState::ZTest::Off;
            render_state.zWrite = RenderState::ZWrite::Off;

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
    vec4 u_rcp_frame;
} buf_0_1;

Input(0) vec2 v_uv;

Output(0) vec4 o_frag;

void main()
{
    o_frag = FxaaPixelShader(v_uv,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),		// FxaaFloat4 fxaaConsolePosPos,
        u_texture,							    // FxaaTex tex,
        u_texture,							    // FxaaTex fxaaConsole360TexExpBiasNegOne,
        u_texture,							    // FxaaTex fxaaConsole360TexExpBiasNegTwo,
        buf_0_1.u_rcp_frame.xy,					// FxaaFloat2 fxaaQualityRcpFrame,
        FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),		// FxaaFloat4 fxaaConsoleRcpFrameOpt,
        FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),		// FxaaFloat4 fxaaConsoleRcpFrameOpt2,
        FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),		// FxaaFloat4 fxaaConsole360RcpFrameOpt2,
        0.75f,									// FxaaFloat fxaaQualitySubpix,
        0.166f,									// FxaaFloat fxaaQualityEdgeThreshold,
        0.0833f,								// FxaaFloat fxaaQualityEdgeThresholdMin,
        0.0f,									// FxaaFloat fxaaConsoleEdgeSharpness,
        0.0f,									// FxaaFloat fxaaConsoleEdgeThreshold,
        0.0f,									// FxaaFloat fxaaConsoleEdgeThresholdMin,
        FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f)		// FxaaFloat fxaaConsole360ConstDir,
    );
}
)";
            
            auto shader = RefMake<Shader>(
                "",
                Vector<String>(),
                vs,
                String::Format(
                    "#extension GL_ARB_gpu_shader5: enable\n"
                    "#define FXAA_QUALITY__PRESET %d\n"
                    "#define FXAA_GLSL_130 1",
                    quality),
                Vector<String>({ "FXAA.fs" }),
                fs,
                render_state);
#elif VR_GLES
            String vs = R"(
attribute vec3 a_pos;
attribute vec2 a_uv;

varying vec2 v_uv;

void main()
{
    gl_Position = vec4(a_pos, 1.0);
    v_uv = vec2(a_uv.x, 1.0 - a_uv.y);
}
)";
            String fs = R"(
precision highp float;

uniform sampler2D u_texture;

uniform vec4 u_rcp_frame;

varying vec2 v_uv;

void main()
{
    gl_FragColor = FxaaPixelShader(v_uv,
        FxaaFloat4(0.0, 0.0, 0.0, 0.0),             // FxaaFloat4 fxaaConsolePosPos,
        u_texture,                                  // FxaaTex tex,
        u_texture,                                  // FxaaTex fxaaConsole360TexExpBiasNegOne,
        u_texture,                                  // FxaaTex fxaaConsole360TexExpBiasNegTwo,
        u_rcp_frame.xy,                             // FxaaFloat2 fxaaQualityRcpFrame,
        FxaaFloat4(0.0, 0.0, 0.0, 0.0),             // FxaaFloat4 fxaaConsoleRcpFrameOpt,
        FxaaFloat4(0.0, 0.0, 0.0, 0.0),             // FxaaFloat4 fxaaConsoleRcpFrameOpt2,
        FxaaFloat4(0.0, 0.0, 0.0, 0.0),             // FxaaFloat4 fxaaConsole360RcpFrameOpt2,
        0.75,                                       // FxaaFloat fxaaQualitySubpix,
        0.166,                                      // FxaaFloat fxaaQualityEdgeThreshold,
        0.0833,                                     // FxaaFloat fxaaQualityEdgeThresholdMin,
        0.0,                                        // FxaaFloat fxaaConsoleEdgeSharpness,
        0.0,                                        // FxaaFloat fxaaConsoleEdgeThreshold,
        0.0,                                        // FxaaFloat fxaaConsoleEdgeThresholdMin,
        FxaaFloat4(0.0, 0.0, 0.0, 0.0)              // FxaaFloat fxaaConsole360ConstDir,
        );
}
)";
            
            auto shader = RefMake<Shader>(
                "",
                Vector<String>(),
                vs,
                String::Format(
                    "#define FXAA_QUALITY__PRESET %d\n"
                    "#define FXAA_GLSL_100 1\n"
                    "#define FXAA_GATHER4_ALPHA 0",
                    quality),
                Vector<String>({ "FXAA.fs" }),
                fs,
                render_state);
#endif

            auto material = RefMake<Material>(shader);
            material->SetVector("u_rcp_frame", Vector4(1.0f / color_texture->GetWidth(), 1.0f / color_texture->GetHeight()));
            material->SetTexture("u_texture", color_texture);

            // color -> window
            m_blit_camera = Display::Instance()->CreateBlitCamera(1, material);

            m_ui_camera->SetDepth(2);
        }

        void InitUI()
        {
            auto canvas = RefMake<CanvasRenderer>();
            m_ui_camera->AddRenderer(canvas);

            // fxaa on/off
            auto label = RefMake<Label>();
            canvas->AddView(label);

            label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetPivot(Vector2(0, 0.5f));
            label->SetSize(Vector2i(100, 30));
            label->SetOffset(Vector2i(40, 120));
            label->SetFont(Font::GetFont(FontType::Consola));
            label->SetFontSize(28);
            label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetText("FXAA");

            auto on = Texture::LoadTexture2DFromFile(Application::Instance()->GetDataPath() + "/texture/ui/switch_on.png", FilterMode::Linear, SamplerAddressMode::ClampToEdge, false, false);
            auto off = Texture::LoadTexture2DFromFile(Application::Instance()->GetDataPath() + "/texture/ui/switch_off.png", FilterMode::Linear, SamplerAddressMode::ClampToEdge, false, false);

            auto switch_button = RefMake<SwitchButton>();
            switch_button->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            switch_button->SetPivot(Vector2(0, 0.5f));
            switch_button->SetSize(Vector2i(182, 57));
            switch_button->SetOffset(Vector2i(130, 120));
            switch_button->SetOnTexture(on);
            switch_button->SetOffTexture(off);
            switch_button->SetSwitchState(true);
            switch_button->SetOnSwitchStateChange([this](bool on) {
                this->OnSwitch(on);
            });

            canvas->AddView(switch_button);

            // quality
            label = RefMake<Label>();
            canvas->AddView(label);

            label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetPivot(Vector2(0, 0.5f));
            label->SetSize(Vector2i(100, 30));
            label->SetOffset(Vector2i(40, 185));
            label->SetFont(Font::GetFont(FontType::Consola));
            label->SetFontSize(28);
            label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetText("Quality");

            auto texture = Texture::LoadTexture2DFromFile(Application::Instance()->GetDataPath() + "/texture/ui/button.png", FilterMode::Linear, SamplerAddressMode::ClampToEdge, false, false);

            auto quality_select = RefMake<SelectButton>();
            quality_select->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            quality_select->SetPivot(Vector2(0, 0.5f));
            quality_select->SetSize(Vector2i(165, 68));
            quality_select->SetOffset(Vector2i(190, 185));
            quality_select->SetTexture(texture);
            quality_select->SetColor(Color(230, 230, 230, 255) / 255.0f);
            quality_select->GetLabel()->SetColor(Color::Black());
            quality_select->SetSelectNames({
                "FAST",
                "DEFAULT",
                "HIGH",
                "EXTREME",
                });
            quality_select->SetSelect(m_target_quality);
            quality_select->SetOnSelectChange([this](int index) {
                this->OnSelectQuality(index);
            });

            canvas->AddView(quality_select);
        }

        void OnSwitch(bool on)
        {
            if (on)
            {
                if (m_blit_camera == nullptr)
                {
                    this->InitRenderTexture();
                }
            }
            else
            {
                m_camera->SetRenderTarget(Ref<Texture>(), Ref<Texture>());
                
                if (m_blit_camera)
                {
                    Display::Instance()->DestroyCamera(m_blit_camera);
                    m_blit_camera = nullptr;
                }
            }
        }

        void OnSelectQuality(int index)
        {
            m_target_quality = index;

            if (m_blit_camera)
            {
                Display::Instance()->DestroyCamera(m_blit_camera);
                m_blit_camera = nullptr;
                this->InitRenderTexture();
            }
        }

        virtual void Init()
        {
            DemoMesh::Init();

            this->InitRenderTexture();
            this->InitUI();
        }

        virtual void Done()
        {
            if (m_blit_camera)
            {
                Display::Instance()->DestroyCamera(m_blit_camera);
                m_blit_camera = nullptr;
            }

            DemoMesh::Done();
        }

        virtual void Update()
        {
            DemoMesh::Update();
        }
    };
}
