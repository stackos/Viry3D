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
    class DemoRenderToTexture : public DemoMesh
    {
    public:
        void InitRenderTexture()
        {
            m_camera->SetDepth(0);
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

            // depth -> color
            auto blit_depth_camera = Display::Instance()->CreateBlitCamera(1, depth_texture, Ref<Material>(), "", CameraClearFlags::Nothing, Rect(0.75f, 0, 0.25f, 0.25f));
            blit_depth_camera->SetRenderTarget(color_texture, Ref<Texture>());

            // color -> window
            Display::Instance()->CreateBlitCamera(0x7fffffff, color_texture);
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

        virtual void OnResize(int width, int height)
        {
            DemoMesh::OnResize(width, height);
        }
    };
}
