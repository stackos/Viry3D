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
#include "Resources.h"
#include "graphics/SkinnedMeshRenderer.h"
#include "animation/Animation.h"
#include "Input.h"

namespace Viry3D
{
    enum class ClipIndex
    {
        Idle,
        Run,
        Shoot,

        Count
    };

    class DemoSkinnedMesh : public DemoMesh
    {
    public:
        Ref<Animation> m_anim;
        Vector<int> m_clips;

        void InitSkinnedMesh()
        {
            // load skinned mesh with animation
            auto node = Resources::Load(Application::Instance()->GetDataPath() + "/res/model/ToonSoldier 1/ToonSoldier 1.go");
            m_anim = RefCast<Animation>(node);

            // set material
            RenderState render_state;

            auto shader = RefMake<Shader>(
                "#define SKINNED_MESH 1",
                Vector<String>({ "Skin.in", "Diffuse.vs.in" }),
                "",
                "",
                Vector<String>({ "Diffuse.fs.in" }),
                "",
                render_state);

            auto texture = Texture::LoadTexture2DFromFile(Application::Instance()->GetDataPath() + "/res/model/ToonSoldier 1/tex.png", FilterMode::Linear, SamplerAddressMode::ClampToEdge, true);

            auto material = RefMake<Material>(shader);
            material->SetTexture("u_texture", texture);
            material->SetVector("u_uv_scale_offset", Vector4(1, 1, 0, 0));
            material->SetColor("u_ambient_color", m_light_param.ambient_color);
            material->SetColor("u_light_color", m_light_param.light_color);
            material->SetVector("u_light_dir", m_light_param.light_rot * Vector3(0, 0, 1));
            material->SetFloat("u_light_intensity", m_light_param.light_intensity);

            auto skin = RefCast<SkinnedMeshRenderer>(m_anim->Find("MESH_Infantry"));
            skin->SetMaterial(material);

            // add to camera
            m_camera->AddRenderer(skin);
            m_renderers.Add(skin);
            
            m_anim->SetLocalPosition(Vector3(0, 0, -0.5f));
            m_anim->SetLocalRotation(Quaternion::Euler(0, 90, 0));

            // play animation clip
            m_clips.Resize((int) ClipIndex::Count, -1);

            int clip_count = m_anim->GetClipCount();
            for (int i = 0; i < clip_count; ++i)
            {
                const String& name = m_anim->GetClipName(i);

                if (name == "assault_combat_idle")
                {
                    m_clips[(int) ClipIndex::Idle] = i;
                }
                else if (name == "assault_combat_run")
                {
                    m_clips[(int) ClipIndex::Run] = i;
                }
                else if (name == "assault_combat_shoot")
                {
                    m_clips[(int) ClipIndex::Shoot] = i;
                }
            }

            m_anim->Play(m_clips[(int) ClipIndex::Idle], 0.3f);
        }

        virtual void Init()
        {
            DemoMesh::Init();

            this->InitSkinnedMesh();
        }

        virtual void Done()
        {
            m_anim.reset();

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
                    m_anim->Play(m_clips[(int) ClipIndex::Run], 0.3f);
                }
                else if (touch.phase == TouchPhase::Ended)
                {
                    m_anim->Play(m_clips[(int) ClipIndex::Idle], 0.3f);
                }
            }

            // update bones
            m_anim->Update();
        }
    };
}
