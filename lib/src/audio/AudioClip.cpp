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

                clip = Ref<AudioClip>(new AudioClip());
                clip->m_channel = wav.channel;
                clip->m_sample_rate = wav.sample_rate;
                clip->m_byte_rate = wav.byte_rate;
                clip->m_sample_bits = wav.sample_bits;
                clip->m_samples = buffer;
            }
        }
        else
        {
            Log("wav file not exist: %s", path.CString());
        }

        return clip;
    }

    AudioClip::AudioClip():
        m_channel(0),
        m_sample_rate(0),
        m_byte_rate(0),
        m_sample_bits(0)
    {
        
    }

    AudioClip::~AudioClip()
    {
        
    }
}
