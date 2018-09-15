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

#include "AudioClip.h"
#include "io/File.h"
#include "io/MemoryStream.h"
#include "memory/Memory.h"
#include "Debug.h"
#include <AL/al.h>

namespace Viry3D
{
    struct WaveHeader
    {
        char riff[4];
        int size;
        char wave[4];
        char fmt[4];
        int wave_size;
        short format;
        short channel;
        int sample_rate;
        int byte_rate;
        short block_align;
        short sample_bits;
    };

    class AudioClipPrivate
    {
    public:
        ALuint m_buffer;

        AudioClipPrivate():
            m_buffer(0)
        {
            alGenBuffers(1, &m_buffer);
        }

        ~AudioClipPrivate()
        {
            alDeleteBuffers(1, &m_buffer);
        }

        void BufferData(int channel, int sample_bits, const void* data, int size, int frequency)
        {
            ALenum format;

            if (channel == 1)
            {
                if (sample_bits == 8)
                {
                    format = AL_FORMAT_MONO8;
                }
                else if (sample_bits == 16)
                {
                    format = AL_FORMAT_MONO16;
                }
                else
                {
                    Log("audio data sample bits error: %d", sample_bits);
                }
            }
            else if (channel == 2)
            {
                if (sample_bits == 8)
                {
                    format = AL_FORMAT_STEREO8;
                }
                else if (sample_bits == 16)
                {
                    format = AL_FORMAT_STEREO16;
                }
                else
                {
                    Log("audio data sample bits error: %d", sample_bits);
                }
            }
            else
            {
                Log("audio data channel error: %d", channel);
            }

            alBufferData(m_buffer, format, data, size, frequency);
        }
    };

    Ref<AudioClip> AudioClip::LoadWaveFromFile(const String& path)
    {
        Ref<AudioClip> clip;

        if (File::Exist(path))
        {
            MemoryStream ms(File::ReadAllBytes(path));

            WaveHeader wav;
            ms.Read(&wav, sizeof(wav));

            if (Memory::Compare(wav.riff, "RIFF", 4) != 0 ||
                Memory::Compare(wav.wave, "WAVE", 4) != 0 ||
                Memory::Compare(wav.fmt, "fmt ", 4) != 0 ||
                wav.format != 1)
            {
                assert(!"wav file format error");
            }

            if (wav.wave_size == 18)
            {
                ms.Read<short>();
            }
            else if (wav.wave_size != 16)
            {
                assert(!"wav file format error");
            }

            char chunk[4];
            ms.Read(chunk, sizeof(chunk));

            while (Memory::Compare(chunk, "data", 4) != 0)
            {
                int size = ms.Read<int>();
                ms.Read(nullptr, size);
                ms.Read(chunk, sizeof(chunk));
            }

            if (Memory::Compare(chunk, "data", 4) == 0)
            {
                int size = ms.Read<int>();
                ByteBuffer buffer(size);
                ms.Read(buffer.Bytes(), buffer.Size());

                int bytes_per_sample = wav.sample_bits / 8 * wav.channel;
                int byte_rate = wav.sample_rate * bytes_per_sample;
                assert(byte_rate == wav.byte_rate);
                float length = size / (float) byte_rate;
                int sample_count = size / bytes_per_sample;

                clip = Ref<AudioClip>(new AudioClip());
                clip->m_channel = wav.channel;
                clip->m_sample_rate = wav.sample_rate;
                clip->m_byte_rate = wav.byte_rate;
                clip->m_sample_bits = wav.sample_bits;
                clip->m_samples = buffer;
                clip->m_length = length;
                clip->m_sample_count = sample_count;
            }
        }
        else
        {
            Log("wav file not exist: %s", path.CString());
        }

        if (clip)
        {
            clip->m_private->BufferData(clip->m_channel, clip->m_sample_bits, clip->m_samples.Bytes(), clip->m_samples.Size(), clip->m_sample_rate);
        }

        return clip;
    }

    AudioClip::AudioClip():
        m_private(new AudioClipPrivate()),
        m_channel(0),
        m_sample_rate(0),
        m_byte_rate(0),
        m_sample_bits(0),
        m_length(0),
        m_sample_count(0)
    {
        
    }

    AudioClip::~AudioClip()
    {
        Memory::SafeDelete(m_private);
    }

    void* AudioClip::GetBuffer() const
    {
        return (void*) (size_t) m_private->m_buffer;
    }
}
