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

namespace Viry3D
{
    class DemoSkybox : public DemoMesh
    {
    public:
        MeshRenderer* m_renderer_sky = nullptr;
        bool m_async_load_complete = false;
        Vector2 m_last_touch_pos;

        void InitSkybox()
        {
            auto cube = Mesh::LoadFromFile(Application::Instance()->GetDataPath() + "/Library/unity default resources.Cube.mesh");

#if VR_VULKAN
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

Input(0) vec3 a_pos;

Output(0) vec3 v_uv;

void main()
{
	gl_Position = (vec4(a_pos, 1.0) * buf_1_0.u_model_matrix * buf_0_0.u_view_matrix * buf_0_0.u_projection_matrix).xyww;
	v_uv = a_pos.xyz;

	vulkan_convert();
}
)";
            String fs = R"(
precision highp float;
      
UniformTexture(0, 1) uniform samplerCube u_texture;

Input(0) vec3 v_uv;

Output(0) vec4 o_frag;

void main()
{
    vec4 c = textureLod(u_texture, v_uv, 0.0);
    o_frag = c;
}
)";
#elif VR_GLES
            String vs = R"(
uniform mat4 u_view_matrix;
uniform mat4 u_projection_matrix;
uniform mat4 u_model_matrix;

attribute vec3 a_pos;

varying vec3 v_uv;

void main()
{
	gl_Position = (vec4(a_pos, 1.0) * u_model_matrix * u_view_matrix * u_projection_matrix).xyww;
	v_uv = a_pos.xyz;
}
)";
            String fs = R"(
precision highp float;
      
uniform samplerCube u_texture;

varying vec3 v_uv;

void main()
{
    vec4 c = textureCube(u_texture, v_uv);
    gl_FragColor = c;
}
)";
#endif
            RenderState render_state;
            render_state.cull = RenderState::Cull::Off;
            render_state.zTest = RenderState::ZTest::Off;
            render_state.zWrite = RenderState::ZWrite::Off;
            render_state.queue = (int) RenderState::Queue::Background;

            auto shader = RefMake<Shader>(
                "",
                Vector<String>(),
                vs,
                "",
                Vector<String>(),
                fs,
                render_state);

            auto material = RefMake<Material>(shader);

            auto renderer = RefMake<MeshRenderer>();
            renderer->SetMaterial(material);
            renderer->SetMesh(cube);
            m_renderer_sky = renderer.get();

            renderer->SetLocalPosition(m_camera_param.pos);

            m_camera_param.rot = Vector3(0, 0, 0);
            m_camera->SetLocalRotation(Quaternion::Euler(m_camera_param.rot));

            Thread::Task task;
            task.job = []() {
                auto cubemap = Texture::CreateCubemap(1024, TextureFormat::R8G8B8A8, FilterMode::Linear, SamplerAddressMode::ClampToEdge, false);
                for (int i = 0; i < 6; ++i)
                {
                    int width;
                    int height;
                    int bpp;
                    ByteBuffer pixels = Texture::LoadImageFromFile(String::Format((Application::Instance()->GetDataPath() + "/texture/env/dawn/%d.png").CString(), i), width, height, bpp);
                    cubemap->UpdateCubemap(pixels, (CubemapFace) i, 0);
                }
                return cubemap;
            };
            task.complete = [=](const Ref<Object>& res) {
                material->SetTexture("u_texture", RefCast<Texture>(res));
                m_camera->AddRenderer(renderer);
                m_async_load_complete = true;
            };
#if VR_WASM
            task.complete(task.job());
#elif VR_VULKAN
            Application::Instance()->GetThreadPool()->AddTask(task);
#elif VR_GLES
            Application::Instance()->GetResourceThreadPool()->AddTask(task);
#endif
        }

        virtual void Init()
        {
            this->InitCamera();
            this->InitUI();
            this->InitSkybox();
        }

        virtual bool IsInitComplete() const
        {
            return m_async_load_complete;
        }

        virtual void Done()
        {
            DemoMesh::Done();
        }

        virtual void Update()
        {
            DemoMesh::Update();

            if (Input::GetTouchCount() > 0)
            {
                const Touch& touch = Input::GetTouch(0);
                if (touch.phase == TouchPhase::Began)
                {
                    m_last_touch_pos = touch.position;
                }
                else if (touch.phase == TouchPhase::Moved)
                {
                    Vector2 delta = touch.position - m_last_touch_pos;
                    m_last_touch_pos = touch.position;

                    m_camera_param.rot.y += -delta.x * 0.1f;
                    m_camera_param.rot.x += delta.y * 0.1f;
                    m_camera->SetLocalRotation(Quaternion::Euler(m_camera_param.rot));
                }
            }
        }
    };
}
