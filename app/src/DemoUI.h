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

namespace Viry3D
{
    class DemoUI : public DemoMesh
    {
    public:
        void InitCanvas()
        {
            auto canvas = RefMake<CanvasRenderer>();
            m_ui_camera->AddRenderer(canvas);

            String text = UR"(国风·卫风·淇奥
瞻彼淇奥，绿竹猗猗。有匪君子，如切如磋，如琢如磨。
瑟兮僩兮，赫兮咺兮。有匪君子，终不可谖兮。
瞻彼淇奥，绿竹青青。有匪君子，充耳璓莹，会弁如星。
瑟兮僩兮，赫兮咺兮。有匪君子，终不可谖兮。
瞻彼淇奥，绿竹如箦。有匪君子，如金如锡，如圭如璧。
宽兮绰兮，猗重较兮。善戏谑兮，不为虐兮。

Vulkan is a new generation graphics and compute API that provides high-efficiency, 
cross-platform access to modern GPUs used in a wide variety of devices from PCs
and consoles to mobile phones and embedded platforms. )";
            auto label = RefMake<Label>();
            canvas->AddView(label);

            label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetPivot(Vector2(0, 0));
            label->SetSize(Vector2i(100, 30));
            label->SetOffset(Vector2i(40, 100));
            label->SetFont(Font::GetFont(FontType::PingFangSC));
            label->SetFontSize(26);
            label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetText(text);

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
                button->SetSize(Vector2i(165, 68));
                button->SetOffset(Vector2i(-500 + i * 200, 100));
                button->SetTexture(texture);
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

                canvas->AddView(button);
            }
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
        }
    };
}
