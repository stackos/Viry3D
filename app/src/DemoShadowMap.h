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

#include "DemoSkinnedMesh.h"

#define SHADOW_MAP_SIZE 1024

namespace Viry3D
{
    class DemoShadowMap : public DemoSkinnedMesh
    {
    public:
        struct ShadowParam
        {
            float ortho_size;
            float near_clip;
            float far_clip;
        };
        ShadowParam m_shadow_param = {
            2.0f,
            -5,
            5
        };

        Camera* m_shadow_camera = nullptr;
        Camera* m_blit_depth_camera = nullptr;
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
                Texture::ChooseDepthFormatSupported(true),
                1,
                1,
                true,
                FilterMode::Nearest,
                SamplerAddressMode::ClampToEdge);
            Ref<Texture> shadow_color_texture = Texture::CreateRenderTexture(
                SHADOW_MAP_SIZE,
                SHADOW_MAP_SIZE,
                TextureFormat::R8G8B8A8,
                1,
                1,
                false,
                FilterMode::None,
                SamplerAddressMode::None);
            
            m_shadow_camera = Display::Instance()->CreateCamera();
            m_shadow_camera->SetDepth(0);
            m_shadow_camera->SetClearFlags(CameraClearFlags::Depth);
            m_shadow_camera->SetRenderTarget(shadow_color_texture, m_shadow_texture);
            m_shadow_camera->SetLocalPosition(m_light->GetPosition());
            m_shadow_camera->SetLocalRotation(m_light->GetRotation());
            m_shadow_camera->SetNearClip(m_shadow_param.near_clip);
            m_shadow_camera->SetFarClip(m_shadow_param.far_clip);
            m_shadow_camera->SetOrthographic(true);
            m_shadow_camera->SetOrthographicSize(m_shadow_param.ortho_size);

            RenderState render_state;
            render_state.cull = RenderState::Cull::Front;

#if VR_VULKAN
            auto shader = RefMake<Shader>(
                "#define CAST_SHADOW 1",
                Vector<String>({ "Diffuse.vs" }),
                "",
                "#define CAST_SHADOW 1",
                Vector<String>({ "Diffuse.fs" }),
                "",
                render_state);
            auto skin_shader = RefMake<Shader>(
                "#define CAST_SHADOW 1\n"
                "#define SKINNED_MESH 1",
                Vector<String>({ "Skin.vs", "Diffuse.vs" }),
                "",
                "#define CAST_SHADOW 1",
                Vector<String>({ "Diffuse.fs" }),
                "",
                render_state);
#elif VR_GLES
            auto shader = RefMake<Shader>(
                "#define CAST_SHADOW 1",
                Vector<String>({ "Diffuse.100.vs" }),
                "",
                "#define CAST_SHADOW 1",
                Vector<String>({ "Diffuse.100.fs" }),
                "",
                render_state);
            auto skin_shader = RefMake<Shader>(
                "#define CAST_SHADOW 1\n"
                "#define SKINNED_MESH 1",
                Vector<String>({ "Diffuse.100.vs" }),
                "",
                "#define CAST_SHADOW 1",
                Vector<String>({ "Diffuse.100.fs" }),
                "",
                render_state);
#endif

            auto material = RefMake<Material>(shader);
            auto skin_material = RefMake<Material>(skin_shader);

            m_light_view_projection_matrix = m_shadow_camera->GetProjectionMatrix() * m_shadow_camera->GetViewMatrix();

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
                shadow_mesh->SetLocalPosition(m_renderers[i]->GetPosition());
                shadow_mesh->SetLocalRotation(m_renderers[i]->GetRotation());
                shadow_mesh->SetLocalScale(m_renderers[i]->GetScale());

                m_shadow_renderers[i] = shadow_mesh;
                m_shadow_camera->AddRenderer(shadow_mesh);
            }

            m_blit_depth_camera = Display::Instance()->CreateBlitCamera(3, m_shadow_texture, CameraClearFlags::Nothing, Rect(0.75f, 0, 0.25f, 0.25f));
        }

        void InitShadowReciever()
        {
            RenderState render_state;

#if VR_VULKAN
            auto shader = RefMake<Shader>(
                "#define RECIEVE_SHADOW 1",
                Vector<String>({ "Diffuse.vs" }),
                "",
                "#define RECIEVE_SHADOW 1",
                Vector<String>({ "Shadow.fs", "Diffuse.fs" }),
                "",
                render_state);
            auto skin_shader = RefMake<Shader>(
                "#define RECIEVE_SHADOW 1\n"
                "#define SKINNED_MESH 1",
                Vector<String>({ "Skin.vs", "Diffuse.vs" }),
                "",
                "#define RECIEVE_SHADOW 1",
                Vector<String>({ "Shadow.fs", "Diffuse.fs" }),
                "",
                render_state);
#elif VR_GLES
            auto shader = RefMake<Shader>(
                "#define RECIEVE_SHADOW 1",
                Vector<String>({ "Diffuse.100.vs" }),
                "",
                "#define RECIEVE_SHADOW 1\n"
                "#define VERSION_100_ES 1",
                Vector<String>({ "Shadow.fs", "Diffuse.100.fs" }),
                "",
                render_state);
            auto skin_shader = RefMake<Shader>(
                "#define RECIEVE_SHADOW 1\n"
                "#define SKINNED_MESH 1",
                Vector<String>({ "Diffuse.100.vs" }),
                "",
                "#define RECIEVE_SHADOW 1\n"
                "#define VERSION_100_ES 1",
                Vector<String>({ "Shadow.fs", "Diffuse.100.fs" }),
                "",
                render_state);
#endif

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
            Display::Instance()->DestroyCamera(m_blit_depth_camera);
            m_blit_depth_camera = nullptr;

            DemoSkinnedMesh::Done();
        }

        virtual void Update()
        {
            DemoSkinnedMesh::Update();
        }
    };
}
