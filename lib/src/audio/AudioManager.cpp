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

#include "AudioManager.h"
#include "AudioListener.h"
#include "memory/Memory.h"
#include "Debug.h"

#if VR_MAC || VR_IOS
#include <OpenAL/OpenAL.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

#if VR_WASM
#include <emscripten.h>

EM_JS(void, PlayAudio, (const char* msg), {
    PlayAudio(UTF8ToString(msg));
});

EM_JS(void, PauseAudio, (const char* msg), {
    PauseAudio(UTF8ToString(msg));
});

EM_JS(void, StopAudio, (const char* msg), {
    StopAudio(UTF8ToString(msg));
});
#endif

namespace Viry3D
{
    static ALCdevice* g_device = nullptr;
    static ALCcontext* g_context = nullptr;
    static AudioListener* g_listener = nullptr;

    void AudioManager::Init()
    {
        String name;
        if (alcIsExtensionPresent(nullptr, "ALC_ENUMERATE_ALL_EXT"))
        {
            name = alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER);
        }
        if (name.Empty())
        {
            name = alcGetString(nullptr, ALC_DEVICE_SPECIFIER);
        }
        Log("open al device specifier: %s", name.CString());

        g_device = alcOpenDevice(nullptr);
        if (g_device == nullptr)
        {
            Log("open al device open failed");
            return;
        }

        g_context = alcCreateContext(g_device, nullptr);
        if (g_context == nullptr)
        {
            Log("open al context create failed");
            return;
        }

        if (alcMakeContextCurrent(g_context) == ALC_FALSE)
        {
            alcDestroyContext(g_context);
            g_context = nullptr;
            alcCloseDevice(g_device);
            g_device = nullptr;
            Log("open al context set failed");
            return;
        }

        Log("open al init success");
    }

    void AudioManager::Done()
    {
        Memory::SafeDelete(g_listener);

        alcMakeContextCurrent(nullptr);
        if (g_context)
        {
            alcDestroyContext(g_context);
            g_context = nullptr;
        }
        if (g_device)
        {
            alcCloseDevice(g_device);
            g_device = nullptr;
        }
    }

    AudioListener* AudioManager::GetListener()
    {
        if (g_listener == nullptr)
        {
            g_listener = new AudioListener();
        }
        return g_listener;
    }

#if VR_WASM
    void AudioManager::PlayAudio(const String& url, bool loop)
    {
        String msg = String::Format(R"({ "url": "%s", "loop": %s })", url.CString(), loop ? "true" : "false");
        ::PlayAudio(msg.CString());
    }

    void AudioManager::PauseAudio()
    {
        ::PauseAudio("");
    }

    void AudioManager::StopAudio()
    {
        ::StopAudio("");
    }
#endif
}
