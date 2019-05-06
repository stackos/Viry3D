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

#include "Object.h"
#include "memory/ByteBuffer.h"
#include <functional>

#define STREAM_BUFFER_MAX 100

namespace Viry3D
{
    class AudioClipPrivate;

    class AudioClip : public Object
    {
    public:
        static Ref<AudioClip> LoadWaveFromFile(const String& path);
#if !VR_WASM
        static Ref<AudioClip> LoadMp3FromFile(const String& path);
#endif
        virtual ~AudioClip();
        void* GetBuffer() const;
        bool IsStream() const { return m_stream; }
        void SetStreamLoop(bool loop);
#if !VR_WASM
        Vector<void*> GetStreamBuffers();
        void RunMp3Decoder();
        void StopMp3Decoder();
#endif

    private:
        AudioClip();

    private:
        AudioClipPrivate* m_private;
        int m_channel;
        int m_sample_rate;
        int m_byte_rate;
        int m_sample_bits;
        ByteBuffer m_samples;
        float m_length;
        int m_sample_count;
        bool m_stream;
    };
}
