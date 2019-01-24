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
    class DemoRenderToTexture : public DemoMesh
    {
    public:
        Camera* m_blit_depth_camera = nullptr;
        Camera* m_blit_color_camera = nullptr;

        void InitRenderTexture()
        {
            auto color_texture = Texture::CreateRenderTexture(
                Display::Instance()->GetWidth(),
                Display::Instance()->GetHeight(),
                TextureFormat::R8G8B8A8,
                1,
                1,
                true,
                FilterMode::Nearest,
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

            // depth -> color
            m_blit_depth_camera = Display::Instance()->CreateBlitCamera(1, depth_texture, CameraClearFlags::Nothing, Rect(0.75f, 0, 0.25f, 0.25f));
            m_blit_depth_camera->SetRenderTarget(color_texture, Ref<Texture>());

            // color -> window
            m_blit_color_camera = Display::Instance()->CreateBlitCamera(2, color_texture);

            m_ui_camera->SetDepth(3);
        }

        virtual void Init()
        {
            DemoMesh::Init();

            this->InitRenderTexture();
        }

        virtual void Done()
        {
            Display::Instance()->DestroyCamera(m_blit_depth_camera);
            m_blit_depth_camera = nullptr;
            Display::Instance()->DestroyCamera(m_blit_color_camera);
            m_blit_color_camera = nullptr;

            DemoMesh::Done();
        }

        virtual void Update()
        {
            DemoMesh::Update();
        }
    };
}
