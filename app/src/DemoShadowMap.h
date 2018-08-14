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

#include "DemoSkinnedMesh.h"

#define SHADOW_MAP_SIZE 1024

namespace Viry3D
{
    class DemoShadowMap : public DemoSkinnedMesh
    {
    public:
        struct ShadowParam
        {
            Vector3 light_pos;
            float ortho_size;
            float near_clip;
            float far_clip;
        };
        ShadowParam m_shadow_param = {
            Vector3(0, 0, 0),
            2.0f,
            -5,
            5
        };

        Camera* m_shadow_camera;
        Vector<Ref<MeshRenderer>> m_shadow_renderers;
        Ref<Texture> m_shadow_texture;
        Matrix4x4 m_light_view_projection_matrix;

        void InitShadowCaster()
        {
            m_camera->SetDepth(1);
            m_ui_camera->SetDepth(2);

            m_shadow_texture = Texture::CreateRenderTexture(
                SHADOW_MAP_SIZE,
                SHADOW_MAP_SIZE,
                Display::Instance()->ChooseFormatSupported(
                    { VK_FORMAT_D32_SFLOAT, VK_FORMAT_X8_D24_UNORM_PACK32, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT),
                true,
                VK_FILTER_LINEAR,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

            m_shadow_camera = Display::Instance()->CreateCamera();
            m_shadow_camera->SetDepth(0);
            m_shadow_camera->SetClearFlags(CameraClearFlags::Depth);
            m_shadow_camera->SetRenderTarget(Ref<Texture>(), m_shadow_texture);

            RenderState render_state;
            render_state.cull = RenderState::Cull::Front;

            auto shader = RefMake<Shader>(
                "#define CAST_SHADOW 1",
                Vector<String>({ "Diffuse.vs.in" }),
                "",
                "#define CAST_SHADOW 1",
                Vector<String>({ "Diffuse.fs.in" }),
                "",
                render_state);
            auto material = RefMake<Material>(shader);

            Vector3 light_forward = m_light_param.light_rot * Vector3(0, 0, 1);
            Vector3 light_up = m_light_param.light_rot * Vector3(0, 1, 0);
            Matrix4x4 view = Matrix4x4::LookTo(m_shadow_param.light_pos, light_forward, light_up);

            int target_width = SHADOW_MAP_SIZE;
            int target_height = SHADOW_MAP_SIZE;
            float ortho_size = m_shadow_param.ortho_size;
            float top = ortho_size;
            float bottom = -ortho_size;
            float plane_h = ortho_size * 2;
            float plane_w = plane_h * target_width / target_height;
            Matrix4x4 projection = Matrix4x4::Ortho(-plane_w / 2, plane_w / 2, bottom, top, m_shadow_param.near_clip, m_shadow_param.far_clip);
            m_light_view_projection_matrix = projection * view;

            material->SetMatrix("u_view_matrix", view);
            material->SetMatrix("u_projection_matrix", projection);

            auto skin_shader = RefMake<Shader>(
                "#define CAST_SHADOW 1\n#define SKINNED_MESH 1",
                Vector<String>({ "Skin.in", "Diffuse.vs.in" }),
                "",
                "#define CAST_SHADOW 1",
                Vector<String>({ "Diffuse.fs.in" }),
                "",
                render_state);
            auto skin_material = RefMake<Material>(skin_shader);
            skin_material->SetMatrix("u_view_matrix", view);
            skin_material->SetMatrix("u_projection_matrix", projection);

            m_shadow_renderers.Resize(m_renderers.Size());
            for (int i = 0; i < m_shadow_renderers.Size(); ++i)
            {
                Ref<MeshRenderer> shadow_mesh;
                Ref<SkinnedMeshRenderer> skin = RefCast<SkinnedMeshRenderer>(m_renderers[i]);
                if (skin)
                {
                    Ref<SkinnedMeshRenderer> shadow_skin = RefMake<SkinnedMeshRenderer>();
                    shadow_skin->SetBonePaths(skin->GetBonePaths());
                    shadow_skin->SetBonesRoot(skin->GetBonesRoot());
                    shadow_mesh = shadow_skin;
                    shadow_mesh->SetMaterial(skin_material);
                }
                else
                {
                    shadow_mesh = RefMake<MeshRenderer>();
                    shadow_mesh->SetMaterial(material);
                }
                
                shadow_mesh->SetMesh(m_renderers[i]->GetMesh(), m_renderers[i]->GetSubmesh());
                shadow_mesh->SetLocalPosition(m_renderers[i]->GetLocalPosition());
                shadow_mesh->SetLocalRotation(m_renderers[i]->GetLocalRotation());
                shadow_mesh->SetLocalScale(m_renderers[i]->GetLocalScale());

                m_shadow_renderers[i] = shadow_mesh;
                m_shadow_camera->AddRenderer(shadow_mesh);
            }

            Display::Instance()->CreateBlitCamera(3, m_shadow_texture, Ref<Material>(), "", CameraClearFlags::Nothing, Rect(0.75f, 0, 0.25f, 0.25f));
        }

        void InitShadowReciever()
        {
            RenderState render_state;

            auto shader = RefMake<Shader>(
                "#define RECIEVE_SHADOW 1",
                Vector<String>({ "Diffuse.vs.in" }),
                "",
                "#define RECIEVE_SHADOW 1",
                Vector<String>({ "Shadow.in", "Diffuse.fs.in" }),
                "",
                render_state);

            auto skin_shader = RefMake<Shader>(
                "#define RECIEVE_SHADOW 1\n#define SKINNED_MESH 1",
                Vector<String>({ "Skin.in", "Diffuse.vs.in" }),
                "",
                "#define RECIEVE_SHADOW 1",
                Vector<String>({ "Shadow.in", "Diffuse.fs.in" }),
                "",
                render_state);

            for (int i = 0; i < m_renderers.Size(); ++i)
            {
                auto material = m_renderers[i]->GetMaterial();

                Ref<SkinnedMeshRenderer> skin = RefCast<SkinnedMeshRenderer>(m_renderers[i]);
                if (skin)
                {
                    material->SetShader(skin_shader);
                }
                else
                {
                    material->SetShader(shader);
                }

                material->SetTexture("u_shadow_texture", m_shadow_texture);
                material->SetMatrix("u_light_view_projection_matrix", m_light_view_projection_matrix);
                material->SetFloat("u_shadow_strength", 1.0f);
                material->SetFloat("u_shadow_z_bias", 0.000f);
                material->SetFloat("u_shadow_slope_bias", 0.0001f);
                material->SetFloat("u_shadow_filter_radius", 1.0f / SHADOW_MAP_SIZE * 3);
            }
        }

        virtual void Init()
        {
            DemoSkinnedMesh::Init();

            this->InitShadowCaster();
            this->InitShadowReciever();
        }

        virtual void Done()
        {
            m_shadow_texture.reset();
            m_shadow_renderers.Clear();

            Display::Instance()->DestroyCamera(m_shadow_camera);
            m_shadow_camera = nullptr;

            DemoSkinnedMesh::Done();
        }

        virtual void Update()
        {
            DemoSkinnedMesh::Update();
        }

        virtual void OnResize(int width, int height)
        {
            DemoSkinnedMesh::OnResize(width, height);
        }
    };
}
