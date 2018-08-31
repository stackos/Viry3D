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

            auto texture0 = Texture::LoadTexture2DFromFile(Application::Instance()->GetDataPath() + "/texture/checkflag.png", FilterMode::Linear, SamplerAddressMode::ClampToEdge, false);
            auto texture1 = Texture::LoadTexture2DFromFile(Application::Instance()->GetDataPath() + "/texture/ui/1.png", FilterMode::Linear, SamplerAddressMode::ClampToEdge, false);
            auto texture2 = Texture::LoadTexture2DFromFile(Application::Instance()->GetDataPath() + "/texture/ui/2.png", FilterMode::Linear, SamplerAddressMode::ClampToEdge, false);
            auto texture3 = Texture::LoadTexture2DFromFile(Application::Instance()->GetDataPath() + "/texture/ui/3.png", FilterMode::Linear, SamplerAddressMode::ClampToEdge, false);
            auto texture4 = Texture::LoadTexture2DFromFile(Application::Instance()->GetDataPath() + "/texture/ui/4.png", FilterMode::Linear, SamplerAddressMode::ClampToEdge, false);
            auto texture5 = Texture::LoadTexture2DFromFile(Application::Instance()->GetDataPath() + "/texture/ui/5.png", FilterMode::Linear, SamplerAddressMode::ClampToEdge, false);

            auto sprite = RefMake<Sprite>();
            sprite->SetSize(Vector2i(100, 100));
            sprite->SetTexture(texture0);

            canvas->AddView(sprite);

            sprite = RefMake<Sprite>();
            sprite->SetSize(Vector2i(100, 100));
            sprite->SetOffset(Vector2i(400, 100));
            sprite->SetTexture(texture1);

            canvas->AddView(sprite);

            sprite = RefMake<Sprite>();
            sprite->SetSize(Vector2i(100, 100));
            sprite->SetOffset(Vector2i(-400, -100));
            sprite->SetTexture(texture2);

            canvas->AddView(sprite);

            sprite = RefMake<Sprite>();
            sprite->SetSize(Vector2i(100, 100));
            sprite->SetOffset(Vector2i(400, -100));
            sprite->SetTexture(texture5);
            sprite->SetLocalRotation(Quaternion::Euler(Vector3(0, 0, 45)));

            canvas->AddView(sprite);

            auto button = RefMake<Button>();
            button->SetSize(Vector2i(100, 100));
            button->SetOffset(Vector2i(-400, 100));
            button->GetLabel()->SetText("button");
            button->SetOnClick([]() {
                Log("click button");
            });

            canvas->AddView(button);
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
