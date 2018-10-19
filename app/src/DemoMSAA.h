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
    class DemoMSAA: public DemoMesh
    {
    public:
        Camera* m_blit_camera = nullptr;
        int m_target_sample_count = 64;

        void InitRenderTexture()
        {
            m_target_sample_count = Mathf::Min(m_target_sample_count, Display::Instance()->GetMaxSamples());

            auto color_texture = Texture::CreateRenderTexture(
                Display::Instance()->GetWidth(),
                Display::Instance()->GetHeight(),
                TextureFormat::R8G8B8A8,
                m_target_sample_count,
                true,
                FilterMode::Linear,
                SamplerAddressMode::ClampToEdge);
            auto depth_texture = Texture::CreateRenderTexture(
                Display::Instance()->GetWidth(),
                Display::Instance()->GetHeight(),
                Texture::ChooseDepthFormatSupported(true),
                m_target_sample_count,
                true,
                FilterMode::Linear,
                SamplerAddressMode::ClampToEdge);
            m_camera->SetRenderTarget(color_texture, depth_texture);

            // color -> window
            m_blit_camera = Display::Instance()->CreateBlitCamera(1, color_texture);

            m_ui_camera->SetDepth(2);
        }

        virtual void Init()
        {
            DemoMesh::Init();

           this->InitRenderTexture();
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
