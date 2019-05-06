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

#include "AudioListener.h"
#include "memory/Memory.h"

#if VR_MAC || VR_IOS
#include <OpenAL/OpenAL.h>
#else
#include <AL/al.h>
#endif

namespace Viry3D
{
    class AudioListenerPrivate
    {
    public:
        AudioListenerPrivate()
        {
            this->SetPosition(Vector3(0, 0, 0));
            this->SetVelocity(Vector3(0, 0, 0));
            this->SetOrientation(Vector3(0, 0, 1), Vector3(0, 1, 0));
        }

        ~AudioListenerPrivate()
        {
            
        }

        void SetPosition(const Vector3& pos)
        {
            alListenerfv(AL_POSITION, (const ALfloat*) &pos);
        }

        void SetVelocity(const Vector3& velocity)
        {
            alListenerfv(AL_VELOCITY, (const ALfloat*) &velocity);
        }

        void SetOrientation(const Vector3& forward, const Vector3& up)
        {
            Vector3 orientation[2] = {
                forward,
                up
            };
            alListenerfv(AL_ORIENTATION, (const ALfloat*) &orientation);
        }
    };

    AudioListener::AudioListener():
        m_private(new AudioListenerPrivate())
    {
    
    }

    AudioListener::~AudioListener()
    {
        Memory::SafeDelete(m_private);
    }

    void AudioListener::OnMatrixDirty()
    {
        Vector3 pos = this->GetPosition();
        Vector3 forward = this->GetForward();
        Vector3 up = this->GetUp();

        m_private->SetPosition(pos);
        m_private->SetOrientation(forward, up);
    }
}
