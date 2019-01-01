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

#include "AudioSource.h"
#include "AudioClip.h"
#include "memory/Memory.h"
#include "container/List.h"
#include "thread/ThreadPool.h"
#include "Debug.h"

#if VR_MAC || VR_IOS
#include <OpenAL/OpenAL.h>
#else
#include <AL/al.h>
#endif

namespace Viry3D
{
    class AudioSourcePrivate
    {
    public:
        ALuint m_source;
        bool m_loop;
        List<ALuint> m_stream_buffers;
        bool m_wait_for_play;
        bool m_paused;

        AudioSourcePrivate():
            m_source(0),
            m_loop(false),
            m_wait_for_play(false),
            m_paused(false)
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
            this->Stop();
            this->ClearBuffers();

            alDeleteSources(1, &m_source);
        }

        void ClearBuffers()
        {
            alSourcei(m_source, AL_BUFFER, 0);

            if (m_stream_buffers.Size() > 0)
            {
                Vector<ALuint> buffers(m_stream_buffers.Size());
                for (int i = 0; i < buffers.Size(); ++i)
                {
                    buffers[i] = m_stream_buffers.First();
                    m_stream_buffers.RemoveFirst();
                }

                alDeleteBuffers(buffers.Size(), &buffers[0]);
            }
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
            m_paused = false;
            alSourcePlay(m_source);
        }

        void Pause()
        {
            if (this->GetState() == AudioSource::State::Playing)
            {
                m_paused = true;
                alSourcePause(m_source);
            }
        }

        void Stop()
        {
            m_paused = false;
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

        void QueueBuffers(const Vector<void*>& buffers)
        {
            for (int i = 0; i < buffers.Size(); ++i)
            {
                ALuint buffer = (ALuint) (size_t) buffers[i];
                m_stream_buffers.AddLast(buffer);

                alSourceQueueBuffers(m_source, 1, &buffer);
            }
        }

        void UnqueueBuffers()
        {
            int processed = 0;
            alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processed);
            if (processed > 0)
            {
                Vector<ALuint> buffers_remove(processed);
                for (int i = 0; i < buffers_remove.Size(); ++i)
                {
                    buffers_remove[i] = m_stream_buffers.First();
                    m_stream_buffers.RemoveFirst();
                }

                alSourceUnqueueBuffers(m_source, buffers_remove.Size(), &buffers_remove[0]);
                alDeleteBuffers(buffers_remove.Size(), &buffers_remove[0]);
            }
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
        this->Stop();

        m_clip = clip;

        if (m_clip)
        {
            if (!m_clip->IsStream())
            {
                ALuint buffer = (ALuint) (size_t) m_clip->GetBuffer();
                m_private->SourceBuffer(buffer);
            }
            else
            {
                m_private->SetLoop(false);

                if (m_private->m_loop)
                {
                    m_clip->SetStreamLoop(true);
                }
            }
        }
    }

    void AudioSource::SetLoop(bool loop)
    {
        m_private->m_loop = loop;

        if (m_clip && m_clip->IsStream())
        {
            m_clip->SetStreamLoop(true);
        }
        else
        {
            m_private->SetLoop(loop);
        }
    }

    void AudioSource::Play()
    {
        if (m_clip)
        {
            if (m_clip->IsStream())
            {
#if !VR_WASM
                if (m_private->m_paused)
                {
                    m_private->Play();
                }
                else
                {
                    this->Stop();

                    m_clip->RunMp3Decoder();

                    m_private->m_wait_for_play = true;
                }
#endif
            }
            else
            {
                m_private->Play();
            }
        }
    }

    void AudioSource::Pause()
    {
        m_private->Pause();
    }

    void AudioSource::Stop()
    {
        if (m_clip)
        {
            m_private->Stop();
            m_private->ClearBuffers();

            if (m_clip->IsStream())
            {
#if !VR_WASM
                m_clip->StopMp3Decoder();

                m_private->m_wait_for_play = false;
#endif
            }
        }
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

    void AudioSource::Update()
    {
#if !VR_WASM
        if (m_clip && m_clip->IsStream())
        {
            if (m_private->m_stream_buffers.Size() < STREAM_BUFFER_MAX)
            {
                m_private->QueueBuffers(m_clip->GetStreamBuffers());
            }
            else
            {
                if (m_private->m_wait_for_play)
                {
                    m_private->m_wait_for_play = false;
                    m_private->Play();
                }
            }
            
            if (!m_private->m_wait_for_play)
            {
                m_private->UnqueueBuffers();
            }
        }
#endif
    }
}
