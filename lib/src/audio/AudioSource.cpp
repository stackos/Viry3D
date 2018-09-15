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

#include "AudioSource.h"
#include "AudioClip.h"
#include "memory/Memory.h"
#include <AL/al.h>

namespace Viry3D
{
    class AudioSourcePrivate
    {
    public:
        ALuint m_source;

        AudioSourcePrivate()
        {
            alGenSources(1, &m_source);

            alSourcef(m_source, AL_PITCH, 1.0f);
            alSourcef(m_source, AL_GAIN, 1.0f);

            this->SetLoop(false);
            this->SetPosition(Vector3(0, 0, 0));
            this->SetVelocity(Vector3(0, 0, 0));
            this->SetDirection(Vector3(0, 0, 1));
        }

        ~AudioSourcePrivate()
        {
            alDeleteSources(1, &m_source);
        }

        void SourceBuffer(ALuint buffer)
        {
            alSourcei(m_source, AL_BUFFER, buffer);
        }

        void SetLoop(bool loop)
        {
            alSourcei(m_source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
        }

        void SetPosition(const Vector3& pos)
        {
            alSourcefv(m_source, AL_POSITION, (const ALfloat*) &pos);
        }

        void SetVelocity(const Vector3& velocity)
        {
            alSourcefv(m_source, AL_VELOCITY, (const ALfloat*) &velocity);
        }

        void SetDirection(const Vector3& dir)
        {
            alSourcefv(m_source, AL_DIRECTION, (const ALfloat*) &dir);
        }

        void Play()
        {
            alSourcePlay(m_source);
        }

        void Pause()
        {
            alSourcePause(m_source);
        }

        void Stop()
        {
            alSourceStop(m_source);
        }

        AudioSource::State GetState() const
        {
            ALint state = 0;
            alGetSourcei(m_source, AL_SOURCE_STATE, &state);
            
            switch (state)
            {
                case AL_INITIAL:
                    return AudioSource::State::Initial;
                case AL_PLAYING:
                    return AudioSource::State::Playing;
                case AL_PAUSED:
                    return AudioSource::State::Paused;
                case AL_STOPPED:
                    return AudioSource::State::Stopped;
            }

            return AudioSource::State::Unknown;
        }
    };

    AudioSource::AudioSource():
        m_private(new AudioSourcePrivate())
    {
    
    }

    AudioSource::~AudioSource()
    {
        Memory::SafeDelete(m_private);
    }

    void AudioSource::SetClip(const Ref<AudioClip>& clip)
    {
        m_clip = clip;

        ALuint buffer = 0;
        if (m_clip)
        {
            buffer = (ALuint) (size_t) m_clip->GetBuffer();
        }

        m_private->SourceBuffer(buffer);
    }

    void AudioSource::SetLoop(bool loop)
    {
        m_private->SetLoop(loop);
    }

    void AudioSource::Play()
    {
        m_private->Play();
    }

    void AudioSource::Pause()
    {
        m_private->Pause();
    }

    void AudioSource::Stop()
    {
        m_private->Stop();
    }

    void AudioSource::OnMatrixDirty()
    {
        Vector3 pos = this->GetPosition();
        Vector3 forward = this->GetForward();

        m_private->SetPosition(pos);
        m_private->SetDirection(forward);
    }

    AudioSource::State AudioSource::GetState() const
    {
        return m_private->GetState();
    }
}
