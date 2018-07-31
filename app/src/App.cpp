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
#include "DemoSkybox.h"
#include "DemoRenderToTexture.h"
#include "DemoFXAA.h"
#include "DemoPostEffectBlur.h"
#include "DemoUI.h"
#include "graphics/Display.h"
#include "graphics/Camera.h"
#include "ui/CanvasRenderer.h"
#include "ui/Button.h"
#include "ui/Label.h"

// TODO:
// - mac project
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
        AppImplement()
        {
        
        }

        void Init()
        {
            m_camera = Display::Instance()->CreateCamera();

            this->InitUI();
        }

        void InitUI()
        {
            auto canvas = RefMake<CanvasRenderer>();
            m_camera->AddRenderer(canvas);

            Vector<String> titles({ "Mesh", "Skybox", "RenderToTexture", "FXAA", "PostEffectBlur", "UI" });

#if VR_WINDOWS || VR_MAC
            int button_height = 80;
            int font_size = 20;
#else
            int button_height = 160;
            int font_size = 40;
#endif

            for (int i = 0; i < titles.Size(); ++i)
            {
                auto button = RefMake<Button>();
                canvas->AddView(button);

                button->SetSize(Vector2i(Display::Instance()->GetWidth(), button_height));
                button->SetAlignment(ViewAlignment::HCenter | ViewAlignment::Top);
                button->SetPivot(Vector2(0.5f, 0));
                button->SetOffset(Vector2i(0, 2 + i * (2 + button_height)));
                button->GetLabel()->SetText(titles[i]);
                button->GetLabel()->SetFontSize(font_size);
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
                    m_demo = new DemoSkybox();
                    break;
                case 2:
                    m_demo = new DemoRenderToTexture();
                    break;
                case 3:
                    m_demo = new DemoFXAA();
                    break;
                case 4:
                    m_demo = new DemoPostEffectBlur();
                    break;
                case 5:
                    m_demo = new DemoUI();
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

        void OnResize(int width, int height)
        {
            if (m_demo)
            {
                m_demo->OnResize(width, height);
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

    void App::OnResize(int width, int height)
    {
        m_app->OnResize(width, height);
    }
}
