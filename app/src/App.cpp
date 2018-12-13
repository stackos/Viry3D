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
#include "DemoMSAA.h"
#include "DemoPostEffectBlur.h"
#include "DemoUI.h"
#include "DemoShadowMap.h"
#include "DemoAudio.h"
#if VR_IOS
#include "DemoAR.h"
#endif
#include "DemoInstancing.h"
#include "DemoLightmap.h"
#include "DemoSSAO.h"
#include "DemoVR.h"
#include "DemoComputeStorageImage.h"
#include "DemoComputeStorageBuffer.h"
#include "graphics/Display.h"
#include "graphics/Camera.h"
#include "ui/CanvasRenderer.h"
#include "ui/Button.h"
#include "ui/Label.h"
#include "ui/ScrollView.h"

// TODO:
// - Compute Shader
// - GLTF load
// - IBL PBR
// - GPU Particle
// - Ray Tracing
// - wasm save path for write file

namespace Viry3D
{
    class AppImplement
    {
    private:
        Camera* m_camera = nullptr;
        Demo* m_demo = nullptr;
        Sprite* m_touch_cursor = nullptr;
        Vector2i m_touch_cursor_pos = Vector2i(0, 0);
        CanvasRenderer* m_canvas = nullptr;
        ScrollView* m_scroll = nullptr;

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
            m_canvas = canvas.get();

            this->AddDemoButtons();
            this->AddTouchCursor();
        }

        void AddDemoButtons()
        {
            auto scroll = RefMake<ScrollView>();
            m_canvas->AddView(scroll);
            m_scroll = scroll.get();

            m_scroll->SetAlignment(ViewAlignment::HCenter | ViewAlignment::VCenter);
            m_scroll->SetPivot(Vector2(0.5f, 0.5f));
            m_scroll->SetSize(Vector2i(VIEW_SIZE_FILL_PARENT, VIEW_SIZE_FILL_PARENT));
            m_scroll->SetOffset(Vector2i(0, 0));

            Vector<String> titles({ "Mesh", "SkinnedMesh", "Skybox", "RenderToTexture", "FXAA", "MSAA", "PostEffectBlur", "UI", "ShadowMap", "Audio", "AR", "Instancing & PBR", "Lightmap",
                                    "Deferred Shading & SSAO", "VR", "ComputeStorageImage", "ComputeStorageBuffer" });

            const int top = (int) (90 * UI_SCALE);
            const int button_height = (int) (160 * UI_SCALE);
            const int button_space = 2;
            const int font_size = (int) (40 * UI_SCALE);
            const int content_height = top + (button_height + button_space) * titles.Size();

            m_scroll->SetContentViewSize(Vector2i(VIEW_SIZE_FILL_PARENT, content_height));

            for (int i = 0; i < titles.Size(); ++i)
            {
                auto button = RefMake<Button>();
                m_scroll->GetContentView()->AddSubview(button);

                button->SetSize(Vector2i(VIEW_SIZE_FILL_PARENT, button_height));
                button->SetAlignment(ViewAlignment::HCenter | ViewAlignment::Top);
                button->SetPivot(Vector2(0.5f, 0));
                button->SetOffset(Vector2i(0, top + i * (button_height + button_space)));
                button->GetLabel()->SetText(titles[i]);
                button->GetLabel()->SetFontSize(font_size);
                button->SetOnClick([=]() { this->ClickDemo(i); });

                bool disabled = false;

#if VR_GLES
                if (i == 4)
                {
                    // MARK:
                    // mac opengl 4.1 / 3.2 not support glsl 120, then use opengl 2.1,
                    // but opengl 2.1 not support some feature in fxaa glsl 120 shader,
                    // and gles 2.0 / webgl 1.0 not support too,
                    // so disable fxaa on mac / gles 2.0,
                    // webgl 2.0 has wrong result, disable too.
#if VR_WASM
                    disabled = true;
#else
                    disabled = !Display::Instance()->IsGLESv3();
#endif
                    if (disabled)
                    {
                        button->GetLabel()->SetText("FXAA (disabled on gles2 / webgl)");
                    }
                }

                // need gles3
                if (!Display::Instance()->IsGLESv3())
                {
                    if (i == 5)
                    {
                        disabled = true;
                        button->GetLabel()->SetText(button->GetLabel()->GetText() + " (disabled on gles2)");
                    }
                }

                // vulkan only
                if (i >= 11)
                {
                    disabled = true;
                    button->GetLabel()->SetText(button->GetLabel()->GetText() + " (implement in vulkan only)");
                }
#elif VR_VULKAN
                if (i == 14)
                {
                    disabled = !Display::Instance()->IsSupportMultiview();
                    if (disabled)
                    {
                        button->GetLabel()->SetText("VR (require VK_KHR_multiview extension)");
                    }
                }
#endif

                if (i == 10)
                {
#if VR_IOS
                    disabled = !ARScene::IsSupported();
#else
                    disabled = true;
#endif
                    if (disabled)
                    {
                        button->GetLabel()->SetText("AR (require ARKit)");
                    }
                }

                if (disabled)
                {
                    button->GetLabel()->SetColor(Color(0.8f, 0.8f, 0.8f, 1));
                    button->SetOnClick(nullptr);
                }
            }
        }

        void ClickDemo(int index)
        {
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
                    m_demo = new DemoMSAA();
                    break;
                case 6:
                    m_demo = new DemoPostEffectBlur();
                    break;
                case 7:
                    m_demo = new DemoUI();
                    break;
                case 8:
                    m_demo = new DemoShadowMap();
                    break;
                case 9:
                    m_demo = new DemoAudio();
                    break;
                case 10:
#if VR_IOS
                    m_demo = new DemoAR();
#endif
                    break;
                case 11:
                    m_demo = new DemoInstancing();
                    break;
                case 12:
                    m_demo = new DemoLightmap();
                    break;
                case 13:
                    m_demo = new DemoSSAO();
                    break;
                case 14:
                    m_demo = new DemoVR();
                    break;
                case 15:
                    m_demo = new DemoComputeStorageImage();
                    break;
                case 16:
                    m_demo = new DemoComputeStorageBuffer();
                    break;
                default:
                    break;
            }

            if (m_demo)
            {
                m_canvas->RemoveAllViews();
                m_camera->SetClearFlags(CameraClearFlags::Nothing);
                m_camera->SetDepth(0x7FFFFFFF);
                m_scroll = nullptr;

                this->AddBackButton();
                this->AddTouchCursor();

                m_demo->Init();
            }
        }

        void AddBackButton()
        {
            int button_width = (int) (400 * UI_SCALE);
            int button_height = (int) (160 * UI_SCALE);
            int font_size = (int) (40 * UI_SCALE);

            auto button = RefMake<Button>();
            m_canvas->AddView(button);

            button->SetSize(Vector2i(button_width, button_height));
            button->SetAlignment(ViewAlignment::Right | ViewAlignment::Bottom);
            button->SetPivot(Vector2(1, 1));
            button->GetLabel()->SetText("Back");
            button->GetLabel()->SetFontSize(font_size);
            button->SetOnClick([=]() { this->ClickBack(); });
        }

        void ClickBack()
        {
            if (m_demo->IsInitComplete())
            {
                m_demo->Done();
                delete m_demo;
                m_demo = nullptr;

                m_canvas->RemoveAllViews();
                m_camera->SetClearFlags(CameraClearFlags::ColorAndDepth);
                m_camera->SetDepth(0);
                this->AddDemoButtons();
                this->AddTouchCursor();
            }
        }

        void AddTouchCursor()
        {
            auto texture = Texture::LoadTexture2DFromFile(Application::Instance()->GetDataPath() + "/texture/touch.png", FilterMode::Linear, SamplerAddressMode::ClampToEdge, false, false);

            auto sprite = RefMake<Sprite>();
            m_canvas->AddView(sprite);

            sprite->SetSize(Vector2i(24, 24));
            sprite->SetAlignment(ViewAlignment::Left | ViewAlignment::Bottom);
            sprite->SetPivot(Vector2(0.5f, 0.5f));
            sprite->SetOffset(m_touch_cursor_pos);
            sprite->SetTexture(texture);

            m_touch_cursor = sprite.get();
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

            if (m_touch_cursor)
            {
                if (Input::GetTouchCount() > 0)
                {
                    Vector2 pos = Input::GetTouch(0).position;
                    m_touch_cursor_pos = Vector2i((int) pos.x, (int) -pos.y);
                    m_touch_cursor->SetOffset(m_touch_cursor_pos);
                }
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
} // namespace Viry3D
