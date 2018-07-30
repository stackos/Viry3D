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

#include "Demo.h"
#include "Application.h"
#include "Debug.h"
#include "graphics/Display.h"
#include "graphics/Camera.h"
#include "graphics/Texture.h"
#include "math/Quaternion.h"
#include "time/Time.h"
#include "ui/CanvasRenderer.h"
#include "ui/Sprite.h"
#include "ui/Label.h"
#include "ui/Font.h"
#include "ui/Button.h"

namespace Viry3D
{
    class DemoUI : public Demo
    {
    public:
        Camera* m_camera;
        Label* m_label;

        void InitUI()
        {
            auto canvas = RefMake<CanvasRenderer>();
            m_camera->AddRenderer(canvas);

            auto texture0 = Texture::LoadTexture2DFromFile(Application::Instance()->GetDataPath() + "/texture/ui/0.png", VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, false);
            auto texture1 = Texture::LoadTexture2DFromFile(Application::Instance()->GetDataPath() + "/texture/ui/1.png", VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, false);
            auto texture2 = Texture::LoadTexture2DFromFile(Application::Instance()->GetDataPath() + "/texture/ui/2.png", VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, false);
            auto texture3 = Texture::LoadTexture2DFromFile(Application::Instance()->GetDataPath() + "/texture/ui/3.png", VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, false);
            auto texture4 = Texture::LoadTexture2DFromFile(Application::Instance()->GetDataPath() + "/texture/ui/4.png", VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, false);
            auto texture5 = Texture::LoadTexture2DFromFile(Application::Instance()->GetDataPath() + "/texture/ui/5.png", VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, false);

            auto sprite = RefMake<Sprite>();
            sprite->SetSize(Vector2i(100, 100));
            sprite->SetTexture(texture3);

            canvas->AddView(sprite);

            sprite = RefMake<Sprite>();
            sprite->SetSize(Vector2i(100, 100));
            sprite->SetOffset(Vector2i(400, 100));
            sprite->SetTexture(texture2);

            canvas->AddView(sprite);

            sprite = RefMake<Sprite>();
            sprite->SetSize(Vector2i(100, 100));
            sprite->SetOffset(Vector2i(-400, -100));
            sprite->SetTexture(texture4);

            canvas->AddView(sprite);

            sprite = RefMake<Sprite>();
            sprite->SetSize(Vector2i(100, 100));
            sprite->SetOffset(Vector2i(400, -100));
            sprite->SetTexture(texture1);
            sprite->SetLocalRotation(Quaternion::Euler(Vector3(0, 0, 45)));

            canvas->AddView(sprite);

            auto label = RefMake<Label>();
            label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetPivot(Vector2(0, 0));
            label->SetSize(Vector2i(100, 30));
            label->SetOffset(Vector2i(40, 40));
            label->SetFont(Font::GetFont(FontType::PingFangSC));
            label->SetFontSize(28);
            label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);

            m_label = label.get();

            canvas->AddView(label);

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
            m_camera = Display::Instance()->CreateCamera();

            this->InitUI();
        }

        virtual void Done()
        {
            Display::Instance()->DestroyCamera(m_camera);
            m_camera = nullptr;
        }

        virtual void Update()
        {
            m_label->SetText(String::Format("FPS:%d", Time::GetFPS()));
        }

        virtual void OnResize(int width, int height)
        {

        }
    };
}
