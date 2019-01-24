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

#include "Demo.h"
#include "Application.h"
#include "graphics/Display.h"
#include "graphics/Camera.h"
#include "time/Time.h"
#include "ui/CanvasRenderer.h"
#include "ui/Label.h"
#include "ui/Font.h"
#include "audio/AudioManager.h"
#include "audio/AudioClip.h"
#include "audio/AudioSource.h"
#include "audio/AudioListener.h"

namespace Viry3D
{
    class DemoAudio : public Demo
    {
    public:
        Ref<AudioSource> m_audio_source_click;
        Ref<AudioSource> m_audio_source_back;
#if !VR_WASM
        Ref<AudioSource> m_audio_source_bgm;
#endif
        Camera* m_ui_camera = nullptr;
        Label* m_label = nullptr;

        void InitAudio()
        {
            auto listener = AudioManager::GetListener();
            listener->SetLocalPosition(Vector3(0, 0, 0));
            listener->SetLocalRotation(Quaternion::Euler(0, 0, 0));

            auto clip = AudioClip::LoadWaveFromFile(Application::Instance()->GetDataPath() + "/audio/click.wav");
            m_audio_source_click = RefMake<AudioSource>();
            m_audio_source_click->SetClip(clip);

            clip = AudioClip::LoadWaveFromFile(Application::Instance()->GetDataPath() + "/audio/back.wav");
            m_audio_source_back = RefMake<AudioSource>();
            m_audio_source_back->SetClip(clip);

#if !VR_WASM
            clip = AudioClip::LoadMp3FromFile(Application::Instance()->GetDataPath() + "/audio/bgm.mp3");
            m_audio_source_bgm = RefMake<AudioSource>();
            m_audio_source_bgm->SetClip(clip);
            m_audio_source_bgm->SetLoop(true);
#endif
        }

        void InitUI()
        {
            m_ui_camera = Display::Instance()->CreateCamera();

            auto canvas = RefMake<CanvasRenderer>();
            m_ui_camera->AddRenderer(canvas);

            auto label = RefMake<Label>();
            canvas->AddView(label);

            label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetPivot(Vector2(0, 0));
            label->SetSize(Vector2i(100, 30));
            label->SetOffset(Vector2i(40, 40));
            label->SetFont(Font::GetFont(FontType::Consola));
            label->SetFontSize(28);
            label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);

            m_label = label.get();

            Vector<String> buttons({
                "Play click.wav",
                "Pause click.wav",
                "Play back.wav",
                "Pause back.wav",
                "Play bgm.mp3",
                "Pause bgm.mp3",
                });

            int button_width = (int) (600 * UI_SCALE);
            int button_height = (int) (160 * UI_SCALE);
            int button_distance = (int) (40 * UI_SCALE);
            int font_size = (int) (40 * UI_SCALE);
            int top = -(buttons.Size() * button_height + (buttons.Size() - 1) * button_distance) / 2;

            for (int i = 0; i < buttons.Size(); ++i)
            {
                auto button = RefMake<Button>();
                canvas->AddView(button);

                button->SetSize(Vector2i(button_width, button_height));
                button->SetAlignment(ViewAlignment::HCenter | ViewAlignment::VCenter);
                button->SetPivot(Vector2(0.5f, 0));
                button->SetOffset(Vector2i(0, top + (button_height + button_distance) * i));
                button->GetLabel()->SetText(buttons[i]);
                button->GetLabel()->SetFontSize(font_size);
                button->SetOnClick([=]() {
                    this->ClickButton(i);
                });
            }
        }

        void ClickButton(int index)
        {
            switch (index)
            {
                case 0:
                    m_audio_source_click->Play();
                    break;
                case 1:
                    m_audio_source_click->Pause();
                    break;
                case 2:
                    m_audio_source_back->Play();
                    break;
                case 3:
                    m_audio_source_back->Pause();
                    break;

                case 4:
#if VR_WASM
                    AudioManager::PlayAudio("audio/bgm.mp3", true);
#else

                    m_audio_source_bgm->Play();
#endif
                    break;
                case 5:
#if VR_WASM
                    AudioManager::PauseAudio();
#else
                    m_audio_source_bgm->Pause();
#endif
                    break;
                    
            }
        }

        virtual void Init()
        {
            this->InitAudio();
            this->InitUI();
        }

        virtual void Done()
        {
            if (m_ui_camera)
            {
                Display::Instance()->DestroyCamera(m_ui_camera);
                m_ui_camera = nullptr;
            }

            m_audio_source_click.reset();
            m_audio_source_back.reset();

#if VR_WASM
            AudioManager::StopAudio();
#else
            m_audio_source_bgm.reset();
#endif
        }

        virtual void Update()
        {
#if !VR_WASM
            m_audio_source_bgm->Update();
#endif

            if (m_label)
            {
                m_label->SetText(String::Format("FPS:%d", Time::GetFPS()));
            }
        }
    };
}
