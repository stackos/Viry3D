/*
* Viry3D
* Copyright 2014-2017 by Stack - stackos@qq.com
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
#include "AudioManager.h"
#include "math/Mathf.h"
#include "io/File.h"
#include "time/Time.h"
#include "memory/Memory.h"
#include "thread/Thread.h"
#include "Debug.h"
#include "mad.h"
#include <chrono>

namespace Viry3D
{
	DEFINE_COM_CLASS(AudioSource);

	struct Mp3Buffer
	{
		static const int PCM_BUFFER_SIZE = 8096;
		const byte* data_mp3;
		int data_mp3_size;
		int channel;
		int frequency;
		int bits;
		char data_pcm[PCM_BUFFER_SIZE];
		int data_pcm_pos;
		bool loop;
		bool play;
		AudioSource* source;
		std::thread* thread;
		Mutex* mutex;
		bool exit;

		void WriteSample(byte* bytes, int channel)
		{
			if (data_pcm_pos + 2 <= PCM_BUFFER_SIZE)
			{
				data_pcm[data_pcm_pos] = bytes[0];
				data_pcm[data_pcm_pos + 1] = bytes[1];

				data_pcm_pos += 2;
			}

			if (data_pcm_pos == PCM_BUFFER_SIZE)
			{
				data_pcm_pos = 0;

				const int QUEUED_BUFFER_MAX = 100;
				const int SLEEP_TIME_MS = 100;

				mutex->lock();
				int queued = AudioManager::GetSourceBufferQueued(source);
				mutex->unlock();

				while (queued >= QUEUED_BUFFER_MAX && !exit)
				{
					AudioManager::ProcessSourceBufferQueue(source);
					std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_MS));

					mutex->lock();
					queued = AudioManager::GetSourceBufferQueued(source);
					mutex->unlock();
				}

				if (exit)
				{
					return;
				}

				// queue buffer to al
				mutex->lock();
				auto buffer = AudioManager::CreateBuffer(channel, frequency, bits, data_pcm, PCM_BUFFER_SIZE);
				AudioManager::SetSourceQueueBuffer(source, buffer);
				if (!play)
				{
					play = true;
					AudioManager::PlaySource(source);
				}
				mutex->unlock();
			}
		}
	};

	static mad_flow mp3_header(void* data, const mad_header* header)
	{
		Mp3Buffer* buffer = (Mp3Buffer*) data;

		buffer->frequency = header->samplerate;
		buffer->bits = 16;

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

	static mad_flow mp3_error(
		void* data,
		mad_stream* stream,
		mad_frame* frame)
	{
		Mp3Buffer* buffer = (Mp3Buffer*) data;

		Log("mp3 decoding error 0x%04x (%s) at byte offset %u\n",
			stream->error, mad_stream_errorstr(stream),
			stream->this_frame - buffer->data_mp3);

		/* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */

		return MAD_FLOW_CONTINUE;
	}

	static mad_flow mp3_input(void* data, mad_stream* stream)
	{
		Mp3Buffer* buffer = (Mp3Buffer*) data;

		if (buffer->data_mp3_size <= 0)
			return MAD_FLOW_STOP;

		mad_stream_buffer(stream, buffer->data_mp3, buffer->data_mp3_size);

		if (!buffer->loop)
		{
			buffer->data_mp3_size = 0;
		}

		if (buffer->exit)
		{
			return MAD_FLOW_STOP;
		}

		return MAD_FLOW_CONTINUE;
	}

	static mad_flow mp3_output(
		void* data,
		mad_header const* header,
		mad_pcm* pcm)
	{
		Mp3Buffer* buffer = (Mp3Buffer*) data;

		unsigned int nchannels, nsamples;
		const mad_fixed_t* left_ch;
		const mad_fixed_t* right_ch;

		/* pcm->samplerate contains the sampling frequency */

		nchannels = pcm->channels;
		nsamples = pcm->length;
		left_ch = pcm->samples[0];
		right_ch = pcm->samples[1];

		while (nsamples--)
		{
			int sample;

			/* output sample(s) in 16-bit signed little-endian PCM */
			byte bytes[2];

			sample = mp3_scale_sample(*left_ch++);
			bytes[0] = (sample >> 0) & 0xff;
			bytes[1] = (sample >> 8) & 0xff;

			buffer->WriteSample(bytes, nchannels);

			if (nchannels == 2)
			{
				sample = mp3_scale_sample(*right_ch++);
				bytes[0] = (sample >> 0) & 0xff;
				bytes[1] = (sample >> 8) & 0xff;

				buffer->WriteSample(bytes, nchannels);
			}
		}

		if (buffer->exit)
		{
			return MAD_FLOW_STOP;
		}

		return MAD_FLOW_CONTINUE;
	}

	static void mp3_decode(Mp3Buffer* buffer, String file)
	{
		auto bb = File::ReadAllBytes(file);
		if (bb.Size() > 0)
		{
			buffer->data_mp3 = (const byte*) bb.Bytes();
			buffer->data_mp3_size = bb.Size();
			buffer->data_pcm_pos = 0;

			mad_decoder decoder;
			mad_decoder_init(&decoder, buffer,
				mp3_input, mp3_header, NULL /* filter */, mp3_output,
				mp3_error, NULL /* message */);

			mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
			mad_decoder_finish(&decoder);
		}
	}

	void AudioSource::PlayMp3File(const String& file)
	{
		if (m_clip)
		{
			return;
		}

		if (m_mp3_buffer != NULL)
		{
			Stop();
		}

		if (m_mp3_buffer == NULL)
		{
			auto mp3_buffer = new Mp3Buffer();
			m_mp3_buffer = mp3_buffer;
			m_mp3_file = file;

			Memory::Zero(mp3_buffer, sizeof(Mp3Buffer));
			mp3_buffer->loop = m_loop;
			mp3_buffer->source = this;
			mp3_buffer->mutex = new Mutex();
			mp3_buffer->thread = new std::thread(mp3_decode, (Mp3Buffer*) mp3_buffer, file);
		}
	}

	AudioSource::~AudioSource()
	{
		Stop();

		AudioManager::DeleteSource(this);
		m_source = 0;
	}

	void AudioSource::DeepCopy(const Ref<Object>& source)
	{
		assert(!"can not copy this component");
	}

	void AudioSource::Awake()
	{
		m_source = AudioManager::CreateSource(this);
	}

	void AudioSource::OnTranformChanged()
	{
		AudioManager::SetSourcePosition(this);
	}

	void AudioSource::SetLoop(bool loop)
	{
		if (m_loop != loop)
		{
			m_loop = loop;

			AudioManager::SetSourceLoop(this);
		}
	}

	void AudioSource::SetClip(const String& file)
	{
		SetClip(AudioClip::LoadFromFile(file));
	}

	void AudioSource::SetClip(const Ref<AudioClip>& clip)
	{
		if (!m_mp3_file.Empty())
		{
			return;
		}

		if (m_clip != clip)
		{
			m_clip = clip;

			AudioManager::SetSourceBuffer(this);
		}
	}

	void AudioSource::SetVolume(float volume)
	{
		if (!Mathf::FloatEqual(m_volume, volume))
		{
			m_volume = volume;

			AudioManager::SetSourceVolume(this);
		}
	}

	void AudioSource::SetTime(float time)
	{
		AudioManager::SetSourceOffset(this, time);
	}

	float AudioSource::GetTime()
	{
		return AudioManager::GetSourceOffset(this);
	}

	bool AudioSource::IsPlaying()
	{
		return AudioManager::IsSourcePlaying(this);
	}

	void AudioSource::Play()
	{
		if (m_clip || m_mp3_buffer != NULL)
		{
			AudioManager::PlaySource(this);
		}
		else if (!m_mp3_file.Empty())
		{
			PlayMp3File(m_mp3_file);
		}
	}

	void AudioSource::Pause()
	{
		AudioManager::PauseSource(this);
	}

	void AudioSource::Stop()
	{
		AudioManager::StopSource(this);

		if (m_mp3_buffer != NULL)
		{
			auto mp3_buffer = (Mp3Buffer*) m_mp3_buffer;
			// wait for thread exit
			mp3_buffer->mutex->lock();
			mp3_buffer->exit = true;
			mp3_buffer->mutex->unlock();
			mp3_buffer->thread->join();
			delete mp3_buffer->thread;
			delete mp3_buffer->mutex;
			delete mp3_buffer;
			m_mp3_buffer = NULL;

			AudioManager::DeleteSourceBufferQueue(this);
		}
	}
}
