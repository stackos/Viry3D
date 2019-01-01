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
#include "ui/Slider.h"

namespace Viry3D
{
    class DemoMSAA: public DemoMesh
    {
    public:
        Camera* m_blit_camera = nullptr;
        int m_max_sample_count = 64;
        int m_sample_count = -1;
        int m_sample_count_target = -1;

        void InitRenderTexture()
        {
            m_max_sample_count = Mathf::Min(m_max_sample_count, Display::Instance()->GetMaxSamples());
            if (m_sample_count == -1)
            {
                m_sample_count = m_max_sample_count;
                m_sample_count_target = m_max_sample_count;
            }

            auto color_texture = Texture::CreateRenderTexture(
                Display::Instance()->GetWidth(),
                Display::Instance()->GetHeight(),
                TextureFormat::R8G8B8A8,
                1,
                m_sample_count,
                true,
                FilterMode::Nearest,
                SamplerAddressMode::ClampToEdge);
            auto depth_texture = Texture::CreateRenderTexture(
                Display::Instance()->GetWidth(),
                Display::Instance()->GetHeight(),
                Texture::ChooseDepthFormatSupported(true),
                1,
                m_sample_count,
                true,
                FilterMode::Nearest,
                SamplerAddressMode::ClampToEdge);
            m_camera->SetRenderTarget(color_texture, depth_texture);

            // color -> window
            m_blit_camera = Display::Instance()->CreateBlitCamera(1, color_texture);

            m_ui_camera->SetDepth(2);
        }

        void InitUI()
        {
            if (m_max_sample_count <= 1)
            {
                return;
            }

            auto canvas = RefMake<CanvasRenderer>();
            m_ui_camera->AddRenderer(canvas);

            auto label = RefMake<Label>();
            canvas->AddView(label);

            int y = 120 + 0 * 65;

            label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetPivot(Vector2(0, 0.5f));
            label->SetSize(Vector2i(100, 30));
            label->SetOffset(Vector2i(40, y));
            label->SetFont(Font::GetFont(FontType::Consola));
            label->SetFontSize(28);
            label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetText("SampleCount");

            auto slider = RefMake<Slider>();
            canvas->AddView(slider);

            slider->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            slider->SetPivot(Vector2(0, 0.5f));
            slider->SetSize(Vector2i(200, 30));
            slider->SetOffset(Vector2i(280, y));

            label = RefMake<Label>();
            canvas->AddView(label);

            label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetPivot(Vector2(0, 0.5f));
            label->SetSize(Vector2i(100, 30));
            label->SetOffset(Vector2i(500, y));
            label->SetFont(Font::GetFont(FontType::Consola));
            label->SetFontSize(28);
            label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);

            int level_max = (int) Mathf::Log2((float) m_max_sample_count);
            int level = (int) Mathf::Log2((float) m_sample_count);
            slider->SetProgress(level / (float) level_max);
            slider->SetValueType(Slider::ValueType::Int, Slider::Value(0), Slider::Value(level_max));
            slider->SetOnValueChange([=](const Slider::Value& value) {
                m_sample_count_target = (int) pow(2, value.int_value);
                label->SetText(String::Format("%d", m_sample_count_target));
            });
            label->SetText(String::Format("%d", m_sample_count));
        }

        virtual void Init()
        {
            DemoMesh::Init();

           this->InitRenderTexture();
           this->InitUI();
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

            if (m_sample_count != m_sample_count_target)
            {
                m_sample_count = m_sample_count_target;

                m_camera->SetRenderTarget(Ref<Texture>(), Ref<Texture>());

                if (m_blit_camera)
                {
                    Display::Instance()->DestroyCamera(m_blit_camera);
                    m_blit_camera = nullptr;
                }

                this->InitRenderTexture();
            }
        }
    };
}
