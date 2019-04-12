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
#include "Debug.h"
#include "ui/Sprite.h"
#include "ui/Button.h"
#include "ui/SpriteAtlas.h"
#include "ui/InputField.h"

namespace Viry3D
{
    class DemoUI : public DemoMesh
    {
    public:
        Sprite* m_sprite = nullptr;
        float m_sprite_fill = 0;

        void InitCanvas()
        {
            m_camera->SetClearColor(Color(0.2f, 0.2f, 0.2f, 1));

            auto canvas = RefMake<CanvasRenderer>(FilterMode::Nearest);
            m_ui_camera->AddRenderer(canvas);

            auto group = RefMake<View>();
            group->SetSize(Vector2i(VIEW_SIZE_FILL_PARENT, VIEW_SIZE_FILL_PARENT));
            canvas->AddView(group);

            String text = UR"(Vulkan is a new generation graphics and compute API that provides high-efficiency, cross-platform access to modern GPUs used in a wide variety of devices from PCs and consoles to mobile phones and embedded platforms. )";
            auto label = RefMake<Label>();
            group->AddSubview(label);

            label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetPivot(Vector2(0, 0));
            label->SetSize(Vector2i(250, 250));
            label->SetOffset(Vector2i(40, 100));
            label->SetFont(Font::GetFont(FontType::PingFangSC));
            label->SetFontSize(26);
            label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetText(text);
            label->SetWrapContent(true);

            auto texture = Texture::LoadTexture2DFromFile(Application::Instance()->GetDataPath() + "/texture/ui/button.png", FilterMode::Linear, SamplerAddressMode::ClampToEdge, false, false);

            Color colors[] = {
                Color(201, 48, 44, 255) / 255.0f,
                Color(236, 151, 31, 255) / 255.0f,
                Color(49, 176, 213, 255) / 255.0f,
                Color(68, 157, 68, 255) / 255.0f,
                Color(40, 96, 144, 255) / 255.0f,
                Color(230, 230, 230, 255) / 255.0f,
            };

            for (int i = 0; i < 6; ++i)
            {
                auto button = RefMake<Button>();
                button->SetSize(Vector2i(195, 195));
                button->SetOffset(Vector2i(-500 + i * 200, 100));
                button->SetTexture(texture,
                    Recti(0, 0, texture->GetWidth(), texture->GetHeight()),
                    Vector4(82, 33, 82, 33));
                button->SetSpriteType(SpriteType::Sliced);
                button->SetColor(colors[i]);
                button->GetLabel()->SetText("button");
                if (i == 5)
                {
                    button->GetLabel()->SetColor(Color::Black());
                }
                else
                {
                    button->GetLabel()->SetColor(Color::White());
                }
                button->SetOnClick([=]() {
                    Log("click button: %d", i);
                });

                group->AddSubview(button);
            }

            auto atlas = SpriteAtlas::LoadFromFile(Application::Instance()->GetDataPath() + "/res/SunnyLand/tileset-sliced.atlas");

            auto sprite = RefMake<Sprite>();
            sprite->SetSize(Vector2i(256, 256));
            sprite->SetOffset(Vector2i(0, 0));
            sprite->SetAtlas(atlas);
            sprite->SetSpriteName("tileset-sliced_0");
            sprite->SetSpriteType(SpriteType::Filled);
            sprite->SetFillMethod(SpriteFillMethod::Radial360);
            sprite->SetFillOrigin((int) SpriteOrigin360::Right);
            sprite->SetFillClockWise(false);
            sprite->SetFillAmount(m_sprite_fill);

            group->AddSubview(sprite);

            m_sprite = sprite.get();

            auto input = RefMake<InputField>();
            input->SetSize(Vector2i(300, 40));
            input->SetOffset(Vector2i(0, 270));

            group->AddSubview(input);
        }

        virtual void Init()
        {
            this->InitCamera();
            this->InitUI();
            this->InitCanvas();
        }

        virtual void Done()
        {
            DemoMesh::Done();
        }

        virtual void Update()
        {
            DemoMesh::Update();

            m_sprite_fill += 0.01f;
            if (m_sprite_fill > 1.0f)
            {
                m_sprite_fill = 0;
            }
            m_sprite->SetFillAmount(m_sprite_fill);
        }
    };
}
