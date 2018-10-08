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
#include "Debug.h"
#include "ui/Sprite.h"
#include "ui/Button.h"

namespace Viry3D
{
    class DemoUI : public DemoMesh
    {
    public:
        void InitUI()
        {
            auto canvas = RefMake<CanvasRenderer>();
            m_ui_camera->AddRenderer(canvas);

            auto texture = Texture::LoadTexture2DFromFile(Application::Instance()->GetDataPath() + "/texture/ui/button.png", FilterMode::Linear, SamplerAddressMode::ClampToEdge, false);

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
                button->SetOffset(Vector2i(-500 + i * 200, 0));
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
                button->SetOnClick([]() {
                    Log("click button");
                });

                canvas->AddView(button);
            }
        }

        virtual void Init()
        {
            DemoMesh::Init();

            this->InitUI();
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
