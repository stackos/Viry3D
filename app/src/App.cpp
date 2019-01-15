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
#include "DemoComputeTexelBuffer.h"
#include "DemoPBR.h"
#include "DemoNavigation2D.h"
#include "graphics/Display.h"
#include "graphics/Camera.h"
#include "ui/CanvasRenderer.h"
#include "ui/Button.h"
#include "ui/Label.h"
#include "ui/ScrollView.h"

// TODO:
// - 2D Game Demo
// - PBR optimize
// - Ray Tracing
// - GPU Particle

namespace Viry3D
{
    struct DemoEntry
    {
        String name;
        Action entry;
    };

    class AppImplement
    {
    private:
        Vector<DemoEntry> m_demo_entries;
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

            m_demo_entries = {
                { "Mesh", [this]() { m_demo = new DemoMesh(); } },
                { "SkinnedMesh", [this]() { m_demo = new DemoSkinnedMesh(); } },
                { "ShadowMap", [this]() { m_demo = new DemoShadowMap(); } },
                { "Skybox", [this]() { m_demo = new DemoSkybox(); } },
                { "RenderToTexture", [this]() { m_demo = new DemoRenderToTexture(); } },
                { "PostEffectBlur", [this]() { m_demo = new DemoPostEffectBlur(); } },
                { "FXAA", [this]() { m_demo = new DemoFXAA(); } },
                { "MSAA", [this]() { m_demo = new DemoMSAA(); } },
                { "UI", [this]() { m_demo = new DemoUI(); } },
                { "Navigation2D", [this]() { m_demo = new DemoNavigation2D(); } },
                { "Audio", [this]() { m_demo = new DemoAudio(); } },
#if VR_IOS
                { "AR", [this]() { m_demo = new DemoAR(); } },
#endif
#if VR_VULKAN
                { "Instancing", [this]() { m_demo = new DemoInstancing(); } },
                { "Lightmap", [this]() { m_demo = new DemoLightmap(); } },
                { "SSAO", [this]() { m_demo = new DemoSSAO(); } },
                { "VR", [this]() { m_demo = new DemoVR(); } },
                { "ComputeStorageImage", [this]() { m_demo = new DemoComputeStorageImage(); } },
                { "ComputeStorageBuffer", [this]() { m_demo = new DemoComputeStorageBuffer(); } },
                { "ComputeTexelBuffer", [this]() { m_demo = new DemoComputeTexelBuffer(); } },
                { "PBR", [this]() { m_demo = new DemoPBR(); } },
#endif
            };

            const int top = (int) (90 * UI_SCALE);
            const int button_height = (int) (160 * UI_SCALE);
            const int button_space = 2;
            const int font_size = (int) (40 * UI_SCALE);
            const int content_height = top + (button_height + button_space) * m_demo_entries.Size();

            m_scroll->SetContentViewSize(Vector2i(VIEW_SIZE_FILL_PARENT, content_height));

            for (int i = 0; i < m_demo_entries.Size(); ++i)
            {
                const String& name = m_demo_entries[i].name;

                auto button = RefMake<Button>();
                m_scroll->GetContentView()->AddSubview(button);

                button->SetSize(Vector2i(VIEW_SIZE_FILL_PARENT, button_height));
                button->SetAlignment(ViewAlignment::HCenter | ViewAlignment::Top);
                button->SetPivot(Vector2(0.5f, 0));
                button->SetOffset(Vector2i(0, top + (m_demo_entries.Size() - 1 - i) * (button_height + button_space)));
                button->GetLabel()->SetText(name);
                button->GetLabel()->SetFontSize(font_size);
                button->SetOnClick([=]() { this->ClickDemo(i); });

                bool disabled = false;

#if VR_GLES
                // need gles3
                if (!Display::Instance()->IsGLESv3())
                {
                    if (name == "MSAA")
                    {
                        disabled = true;
                        button->GetLabel()->SetText("MSAA (disabled on gles2)");
                    }
                }
#elif VR_VULKAN
                if (name == "VR")
                {
                    disabled = !Display::Instance()->IsSupportMultiview();
                    if (disabled)
                    {
                        button->GetLabel()->SetText("VR (require VK_KHR_multiview extension)");
                    }
                }
                else if (name == "ComputeTexelBuffer")
                {
#if VR_MAC || VR_IOS
                    disabled = true;
                    button->GetLabel()->SetText("ComputeTexelBuffer (MoltenVK texel buffer support problem)");
#endif
                }
#endif

#if VR_IOS
                if (name == "AR")
                {
                    disabled = !ARScene::IsSupported();
                    if (disabled)
                    {
                        button->GetLabel()->SetText("AR (require ARKit)");
                    }
                }
#endif

                if (disabled)
                {
                    button->GetLabel()->SetColor(Color(0.8f, 0.8f, 0.8f, 1));
                    button->SetOnClick(nullptr);
                }
            }
        }

        void ClickDemo(int index)
        {
            m_demo_entries[index].entry();

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
            auto texture = Texture::LoadTexture2DFromFile(Application::Instance()->GetDataPath() + "/texture/cursor.png", FilterMode::Linear, SamplerAddressMode::ClampToEdge, false, false);

            auto sprite = RefMake<Sprite>();
            m_canvas->AddView(sprite);

            sprite->SetSize(Vector2i(texture->GetWidth(), texture->GetHeight()));
            sprite->SetAlignment(ViewAlignment::Left | ViewAlignment::Bottom);
            sprite->SetPivot(Vector2(0.02f, 0.02f));
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
}
