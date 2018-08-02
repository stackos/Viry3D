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

namespace Viry3D
{
    class DemoShadowMap : public DemoMesh
    {
    public:
        Camera* m_shadow_camera;
        Vector<Ref<MeshRenderer>> m_shadow_renderers;

        void InitShadowCamera()
        {
            m_camera->SetDepth(0);

            const int shadow_map_size = 1024;
            auto shadow_texture = Texture::CreateRenderTexture(
                    shadow_map_size,
                    shadow_map_size,
                    Display::Instance()->ChooseFormatSupported(
                            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_X8_D24_UNORM_PACK32, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT),
                    true,
                    VK_FILTER_LINEAR,
                    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

            m_shadow_camera = Display::Instance()->CreateCamera();
            m_shadow_camera->SetDepth(1);
            m_shadow_camera->SetClearFlags(CameraClearFlags::Depth);
            m_shadow_camera->SetRenderTarget(Ref<Texture>(), shadow_texture);

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

            Vector3 camera_forward = m_camera_param.rot * Vector3(0, 0, 1);
            Vector3 camera_up = m_camera_param.rot * Vector3(0, 1, 0);
            Matrix4x4 view = Matrix4x4::LookTo(m_camera_param.pos, camera_forward, camera_up);
            Matrix4x4 projection = Matrix4x4::Perspective(m_camera_param.fov, shadow_map_size / (float) shadow_map_size, m_camera_param.near_clip, m_camera_param.far_clip);

            material->SetMatrix("u_view_matrix", view);
            material->SetMatrix("u_projection_matrix", projection);

            m_shadow_renderers.Resize(m_renderers.Size());
            for (int i = 0; i < m_shadow_renderers.Size(); ++i)
            {
                auto renderer = RefMake<MeshRenderer>();
                renderer->SetMaterial(material);
                renderer->SetMesh(m_renderers[i]->GetMesh(), m_renderers[i]->GetSubmesh());
                renderer->SetInstanceMatrix("u_model_matrix", *m_renderers[i]->GetInstanceMatrix("u_model_matrix"));

                m_shadow_renderers[i] = renderer;
                m_shadow_camera->AddRenderer(renderer);
            }

            Display::Instance()->CreateBlitCamera(2, shadow_texture, Ref<Material>(), "", CameraClearFlags::Nothing, Rect(0.75f, 0, 0.25f, 0.25f));
        }

        virtual void Init()
        {
            DemoMesh::Init();

            this->InitShadowCamera();
        }

        virtual void Done()
        {
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
