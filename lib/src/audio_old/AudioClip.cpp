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

#include "AudioClip.h"
#include "io/File.h"
#include "io/MemoryStream.h"
#include "memory/Memory.h"
#include "Debug.h"
#include "Application.h"

#if VR_MAC || VR_IOS
#include <OpenAL/OpenAL.h>
#else
#include <AL/al.h>
#endif

#if !VR_WASM
#include "mp3/mad/mad.h"
#endif

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
#if !VR_WASM
        mad_decoder* m_mp3_decoder;
        ByteBuffer m_mp3_buffer;
        int m_mp3_write_size;
        ByteBuffer m_out_buffer;
        List<ALuint> m_stream_buffers;
        bool m_decoder_exit;
        bool m_decoder_exited;
        bool m_decoder_running;
#endif
        ALuint m_buffer;
        Mutex m_mutex;
        bool m_stream_loop;

        AudioClipPrivate():
#if !VR_WASM
            m_mp3_decoder(nullptr),
            m_mp3_write_size(0),
            m_decoder_exit(false),
            m_decoder_exited(false),
            m_decoder_running(false),
#endif
            m_buffer(0),
            m_stream_loop(false)
        {
            
        }

        ~AudioClipPrivate()
        {
#if !VR_WASM
            if (m_mp3_decoder)
            {
                this->StopMp3Decoder();

                mad_decoder_finish(m_mp3_decoder);
                delete m_mp3_decoder;
                m_mp3_decoder = nullptr;
            }
#endif

            if (m_buffer)
            {
                alDeleteBuffers(1, &m_buffer);
                m_buffer = 0;
            }
        }

        static void BufferData(ALuint buffer, int channel, int sample_bits, const void* data, int size, int frequency)
        {
            ALenum format = 0;

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

            alBufferData(buffer, format, data, size, frequency);
        }

        void BufferData(int channel, int sample_bits, const void* data, int size, int frequency)
        {
            if (m_buffer == 0)
            {
                alGenBuffers(1, &m_buffer);
            }

            BufferData(m_buffer, channel, sample_bits, data, size, frequency);
        }

#if !VR_WASM
        void StopMp3Decoder()
        {
            if (m_decoder_running)
            {
                this->ExitMp3Decoder();
                m_decoder_running = false;
            }

            for (auto i : m_stream_buffers)
            {
                alDeleteBuffers(1, &i);
            }
            m_stream_buffers.Clear();
        }

        void InitMp3Decoder(const ByteBuffer& buffer);

        void RunMp3Decoder()
        {
            this->StopMp3Decoder();

            if (!m_decoder_running)
            {
                m_decoder_running = true;
                m_decoder_exit = false;
                m_decoder_exited = false;

                m_mp3_write_size = 0;

                Thread::Task task;
                task.job = [this]() {
                    mad_decoder_run(m_mp3_decoder, MAD_DECODER_MODE_SYNC);

                    m_mutex.lock();
                    m_decoder_exited = true;
                    m_mutex.unlock();

                    return Ref<Object>();
                };
                Application::Instance()->GetThreadPool()->AddTask(task);
            }
        }

        void ExitMp3Decoder()
        {
            m_mutex.lock();
            m_decoder_exit = true;
            m_mutex.unlock();

            this->WaitMp3DecoderExit();
        }

        void WaitMp3DecoderExit()
        {
            bool exist = false;

            while (!exist)
            {
                Thread::Sleep(1);

                m_mutex.lock();
                exist = m_decoder_exited;
                m_mutex.unlock();
            }
        }
#endif
    };

#if !VR_WASM
    static bool is_decode_exit(void* data)
    {
        AudioClipPrivate* clip = (AudioClipPrivate*) data;

        bool exit = false;

        clip->m_mutex.lock();
        exit = clip->m_decoder_exit;
        clip->m_mutex.unlock();

        return exit;
    }

    static mad_flow mp3_input(void* data, mad_stream* stream)
    {
        if (is_decode_exit(data))
        {
            return MAD_FLOW_STOP;
        }

        AudioClipPrivate* clip = (AudioClipPrivate*) data;

        if (clip->m_mp3_write_size < clip->m_mp3_buffer.Size())
        {
            byte* buffer = clip->m_mp3_buffer.Bytes();
            int size = clip->m_mp3_buffer.Size();

            mad_stream_buffer(stream, &buffer[clip->m_mp3_write_size], size);

            clip->m_mp3_write_size += size;

            return MAD_FLOW_CONTINUE;
        }
        else
        {
            bool loop = false;

            clip->m_mutex.lock();
            loop = clip->m_stream_loop;
            clip->m_mutex.unlock();

            if (loop)
            {
                clip->m_mp3_write_size = 0;

                return mp3_input(data, stream);
            }
            else
            {
                return MAD_FLOW_STOP;
            }
        }
    }

    static mad_flow mp3_header(void* data, mad_header const* header)
    {
        if (is_decode_exit(data))
        {
            return MAD_FLOW_STOP;
        }

        return MAD_FLOW_CONTINUE;
    }

    static mad_flow mp3_filter(void* data, mad_stream const* stream, mad_frame* frame)
    {
        if (is_decode_exit(data))
        {
            return MAD_FLOW_STOP;
        }

        return MAD_FLOW_CONTINUE;
    }

    static int mp3_scale_sample(mad_fixed_t sample)
    {
        /* round */
        sample += (1L << (MAD_F_FRACBITS - 16));

        /* clip */
        if (sample >= MAD_F_ONE)
            sample = MAD_F_ONE - 1;
        else if (sample < -MAD_F_ONE)
            sample = -MAD_F_ONE;

        /* quantize */
        return sample >> (MAD_F_FRACBITS + 1 - 16);
    }

    static mad_flow mp3_output(void* data, mad_header const* header, mad_pcm* pcm)
    {
        if (is_decode_exit(data))
        {
            return MAD_FLOW_STOP;
        }

        AudioClipPrivate* clip = (AudioClipPrivate*) data;
        
        int bytes = (int) (header->bitrate / header->samplerate / pcm->channels);
        assert(bytes == 1 || bytes == 2);

        const int max_sample = 1152;
        int size = max_sample * bytes * pcm->channels;

        if (clip->m_out_buffer.Size() == 0)
        {
            clip->m_out_buffer = ByteBuffer(size);
        }

        for (int i = 0; i < pcm->length; ++i)
        {
            for (int j = 0; j < pcm->channels; ++j)
            {
                int sample = mp3_scale_sample(pcm->samples[j][i]);

                if (bytes == 1)
                {
                    byte* p = clip->m_out_buffer.Bytes();

                    p[i * pcm->channels + j] = ((sample + 0x8000) >> 8) & 0xff;
                }
                else if (bytes == 2)
                {
                    short* p = (short*) clip->m_out_buffer.Bytes();

                    unsigned char bytes[2];
                    bytes[0] = (sample >> 0) & 0xff;
                    bytes[1] = (sample >> 8) & 0xff;

                    p[i * pcm->channels + j] = *(short*) bytes;
                }
            }
        }

        ALuint buffer = 0;
        alGenBuffers(1, &buffer);
        AudioClipPrivate::BufferData(buffer, pcm->channels, bytes * 8, clip->m_out_buffer.Bytes(), pcm->length * bytes * pcm->channels, header->samplerate);

        bool exit = false;
        int buffer_count = 0;

        clip->m_mutex.lock();
        clip->m_stream_buffers.AddLast(buffer);
        buffer_count = clip->m_stream_buffers.Size();
        clip->m_mutex.unlock();

        while (buffer_count >= STREAM_BUFFER_MAX)
        {
            Thread::Sleep(100);

            clip->m_mutex.lock();
            exit = clip->m_decoder_exit;
            buffer_count = clip->m_stream_buffers.Size();
            clip->m_mutex.unlock();

            if (exit)
            {
                return MAD_FLOW_STOP;
            }
        }
        
        return MAD_FLOW_CONTINUE;
    }

    static mad_flow mp3_error(void* data, mad_stream* stream, mad_frame* frame)
    {
        return MAD_FLOW_BREAK;
    }

    static mad_flow mp3_message(void* data, void*, unsigned int*)
    {
        if (is_decode_exit(data))
        {
            return MAD_FLOW_STOP;
        }

        return MAD_FLOW_CONTINUE;
    }

    void AudioClipPrivate::InitMp3Decoder(const ByteBuffer& buffer)
    {
        m_mp3_buffer = buffer;
        m_mp3_decoder = new mad_decoder();
        mad_decoder_init(m_mp3_decoder, this, mp3_input, mp3_header, mp3_filter, mp3_output, mp3_error, mp3_message);
    }
#endif

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

#if !VR_WASM
    Ref<AudioClip> AudioClip::LoadMp3FromFile(const String& path)
    {
        Ref<AudioClip> clip;

        if (File::Exist(path))
        {
            ByteBuffer buffer = File::ReadAllBytes(path);
            
            clip = Ref<AudioClip>(new AudioClip());
            clip->m_stream = true;
            clip->m_private->InitMp3Decoder(buffer);
        }
        else
        {
            Log("mp3 file not exist: %s", path.CString());
        }

        return clip;
    }
#endif

    AudioClip::AudioClip():
        m_private(new AudioClipPrivate()),
        m_channel(0),
        m_sample_rate(0),
        m_byte_rate(0),
        m_sample_bits(0),
        m_length(0),
        m_sample_count(0),
        m_stream(false)
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

    void AudioClip::SetStreamLoop(bool loop)
    {
        m_private->m_mutex.lock();
        m_private->m_stream_loop = loop;
        m_private->m_mutex.unlock();
    }

#if !VR_WASM
    Vector<void*> AudioClip::GetStreamBuffers()
    {
        Vector<void*> buffers;

        m_private->m_mutex.lock();
        for (auto i : m_private->m_stream_buffers)
        {
            buffers.Add((void*) (size_t) i);
        }
        m_private->m_stream_buffers.Clear();
        m_private->m_mutex.unlock();

        return buffers;
    }

    void AudioClip::RunMp3Decoder()
    {
        m_private->RunMp3Decoder();
    }

    void AudioClip::StopMp3Decoder()
    {
        m_private->StopMp3Decoder();
    }
#endif
}
