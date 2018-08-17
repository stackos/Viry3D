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

#define FXAA_QUALITY_FAST		10
#define FXAA_QUALITY_DEFAULT	12
#define FXAA_QUALITY_HIGH		29
#define FXAA_QUALITY_EXTREME	39

namespace Viry3D
{
    class DemoFXAA : public DemoMesh
    {
    public:
        void InitRenderTexture()
        {
            auto color_texture = Texture::CreateRenderTexture(
                Display::Instance()->GetWidth(),
                Display::Instance()->GetHeight(),
                VK_FORMAT_R8G8B8A8_UNORM,
                true,
                VK_FILTER_LINEAR,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
            auto depth_texture = Texture::CreateRenderTexture(
                Display::Instance()->GetWidth(),
                Display::Instance()->GetHeight(),
                Display::Instance()->ChooseFormatSupported(
                    { VK_FORMAT_D32_SFLOAT, VK_FORMAT_X8_D24_UNORM_PACK32, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT),
                true,
                VK_FILTER_LINEAR,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
            m_camera->SetRenderTarget(color_texture, depth_texture);

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
            RenderState render_state;
            render_state.cull = RenderState::Cull::Off;
            render_state.zTest = RenderState::ZTest::Off;
            render_state.zWrite = RenderState::ZWrite::Off;

            auto shader = RefMake<Shader>(
                "",
                Vector<String>(),
                vs,
                String::Format("#define FXAA_QUALITY__PRESET %d", FXAA_QUALITY_EXTREME),
                Vector<String>({ "FXAA.in" }),
                fs,
                render_state);
            auto material = RefMake<Material>(shader);
            material->SetVector("u_rcp_frame", Vector4(1.0f / Display::Instance()->GetWidth(), 1.0f / Display::Instance()->GetHeight()));

            // color -> window
            Display::Instance()->CreateBlitCamera(1, color_texture, material);

            m_ui_camera->SetDepth(2);
        }

        virtual void Init()
        {
            DemoMesh::Init();

            this->InitRenderTexture();
        }

        virtual void Done()
        {
            DemoMesh::Done();
        }

        virtual void Update()
        {
            DemoMesh::Update();
        }
    };
}
