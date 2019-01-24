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
#include "graphics/Computer.h"
#include "graphics/BufferObject.h"

namespace Viry3D
{
    class DemoComputeTexelBuffer : public DemoMesh
    {
    public:
        Camera* m_blit_color_camera = nullptr;
        Ref<BufferObject> m_buffer;

        void InitCompute()
        {
#if VR_VULKAN
            String cs = R"(#version 310 es
#extension GL_EXT_texture_buffer : enable
layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
layout (binding = 0, rgba8) uniform writeonly lowp imageBuffer uBuffer;
layout (binding = 1) uniform ImageSize
{
    int uWidth;
    int uHeight;
};

void main() 
{
    ivec2 uv = ivec2(int(gl_GlobalInvocationID.x), int(gl_GlobalInvocationID.y));
    int index = uv.y * uWidth + uv.x;
    vec4 color = vec4(float(uv.x) / float(uWidth), float(uv.y) / float(uHeight), 0, 0);
    imageStore(uBuffer, index, color);
}
)";

            m_buffer = Display::Instance()->CreateBuffer(nullptr, 1024 * 1024 * 2, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, true, VK_FORMAT_R8G8_UNORM);

            auto shader = RefMake<Shader>(cs);
            auto material = RefMake<Material>(shader);
            material->SetInt("uWidth", 1024);
            material->SetInt("uHeight", 1024);
            material->SetStorageTexelBuffer("uBuffer", m_buffer);

            auto computer = RefMake<Computer>();
            computer->SetMaterial(material);
            computer->SetWorkgroupCount(1024 / 16, 1024 / 16, 1);

            m_camera->AddRenderer(computer);

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
#extension GL_OES_texture_buffer : enable
precision highp float;

layout (binding = 0) uniform lowp samplerBuffer uBuffer;
layout (binding = 1) uniform ImageSize
{
    int uWidth;
    int uHeight;
};

Input(0) vec2 v_uv;

Output(0) vec4 o_frag;

void main()
{
    ivec2 uv = ivec2(int(v_uv.x * float(uWidth)), int(v_uv.y * float(uHeight)));
    int index = uv.y * uWidth + uv.x;
    vec4 color = texelFetch(uBuffer, index);
    o_frag = vec4(color.rg, 1.0, 1.0);
}
)";
            RenderState render_state;
            render_state.cull = RenderState::Cull::Off;
            render_state.zTest = RenderState::ZTest::Off;
            render_state.zWrite = RenderState::ZWrite::Off;

            shader = RefMake<Shader>(
                "",
                Vector<String>(),
                vs,
                "",
                Vector<String>(),
                fs,
                render_state);
            material = RefMake<Material>(shader);
            material->SetInt("uWidth", 1024);
            material->SetInt("uHeight", 1024);
            material->SetUniformTexelBuffer("uBuffer", m_buffer);

            m_blit_color_camera = Display::Instance()->CreateBlitCamera(1, material);

            m_ui_camera->SetDepth(2);
#endif
        }

        virtual void Init()
        {
            this->InitCamera();
            this->InitUI();
            this->InitCompute();
        }

        virtual void Done()
        {
            Display::Instance()->DestroyCamera(m_blit_color_camera);
            m_blit_color_camera = nullptr;

            if (m_buffer)
            {
#if VR_VULKAN
                m_buffer->Destroy(Display::Instance()->GetDevice());
#endif
                m_buffer.reset();
            }

            DemoMesh::Done();
        }

        virtual void Update()
        {
            DemoMesh::Update();
        }
    };
}
