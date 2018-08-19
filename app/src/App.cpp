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

#include "App.h"
#include "DemoMesh.h"
#include "DemoSkinnedMesh.h"
#include "DemoSkybox.h"
#include "DemoRenderToTexture.h"
#include "DemoFXAA.h"
#include "DemoPostEffectBlur.h"
#include "DemoUI.h"
#include "DemoShadowMap.h"
#include "graphics/Display.h"
#include "graphics/Camera.h"
#include "ui/CanvasRenderer.h"
#include "ui/Button.h"
#include "ui/Label.h"

// TODO:
// - Light
// - SwitchControl
// - SliderControl
// - ScrollView TabView TreeView

namespace Viry3D
{
    class AppImplement
    {
    private:
        Camera* m_camera = nullptr;
        Demo* m_demo = nullptr;

    public:
        void Init()
        {
            m_camera = Display::Instance()->CreateCamera();

            this->InitUI();
        }

        void InitUI()
        {
            auto canvas = RefMake<CanvasRenderer>();
            m_camera->AddRenderer(canvas);

            Vector<String> titles({ "Mesh", "SkinnedMesh", "Skybox", "RenderToTexture", "FXAA", "PostEffectBlur", "UI", "ShadowMap" });

#if VR_WINDOWS || VR_MAC
            float scale = 0.4f;
#else
            float scale = 1.0f;
#endif

            int top = 90;
            int button_height = 160;
            int font_size = 40;

            for (int i = 0; i < titles.Size(); ++i)
            {
                auto button = RefMake<Button>();
                canvas->AddView(button);

                button->SetSize(Vector2i(Display::Instance()->GetWidth(), (int) (button_height * scale)));
                button->SetAlignment(ViewAlignment::HCenter | ViewAlignment::Top);
                button->SetPivot(Vector2(0.5f, 0));
                button->SetOffset(Vector2i(0, (int) (top * scale) + i * (2 + (int) (button_height * scale))));
                button->GetLabel()->SetText(titles[i]);
                button->GetLabel()->SetFontSize((int) (font_size * scale));
                button->SetOnClick([=]() {
                    this->ClickDemo(i);
                });
            }
        }

        void ClickDemo(int index)
        {
            Display::Instance()->DestroyCamera(m_camera);
            m_camera = nullptr;

            switch (index)
            {
                case 0:
                    m_demo = new DemoMesh();
                    break;
                case 1:
                    m_demo = new DemoSkinnedMesh();
                    break;
                case 2:
                    m_demo = new DemoSkybox();
                    break;
                case 3:
                    m_demo = new DemoRenderToTexture();
                    break;
                case 4:
                    m_demo = new DemoFXAA();
                    break;
                case 5:
                    m_demo = new DemoPostEffectBlur();
                    break;
                case 6:
                    m_demo = new DemoUI();
                    break;
                case 7:
                    m_demo = new DemoShadowMap();
                    break;
                default:
                    break;
            }

            m_demo->Init();
        }

        ~AppImplement()
        {
            if (m_camera)
            {
                Display::Instance()->DestroyCamera(m_camera);
                m_camera = nullptr;
            }

            if (m_demo)
            {
                m_demo->Done();
                delete m_demo;
                m_demo = nullptr;
            }
        }

        void Update()
        {
            if (m_demo)
            {
                m_demo->Update();
            }
        }
    };

    App::App()
    {
        m_app = new AppImplement();
    }

    App::~App()
    {
        delete m_app;
    }

    void App::Init()
    {
        m_app->Init();
    }

    void App::Update()
    {
        m_app->Update();
    }
}
