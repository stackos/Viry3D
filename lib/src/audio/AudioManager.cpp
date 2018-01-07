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

#include "AudioManager.h"
#include "AudioListener.h"
#include "AudioClip.h"
#include "AudioSource.h"
#include "GameObject.h"
#include "Debug.h"
#include "container/Vector.h"
#include "container/List.h"
#include "thread/Thread.h"

#ifdef VR_IOS
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#elif VR_MAC
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include "AL/al.h"
#include "AL/alc.h"
#endif

#define OAL_DEVICE_ID 0

namespace Viry3D
{
	typedef std::lock_guard<Mutex> MutexLock;

	static ALCdevice* g_device;
	static ALCcontext* g_context;
	static Mutex g_context_mutex;
	static List<ALuint> g_sources;
	static List<ALuint> g_sources_paused;

	static Vector<String> get_devices()
	{
		Vector<String> devices;

		const ALchar* devicesStr = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
		const ALCchar* device = devicesStr;
		const ALCchar* next = devicesStr + 1;
		int len = 0;

		while (device && *device != 0 && next && *next != 0)
		{
			devices.Add(device);

			len = (int) strlen(device);
			device += (len + 1);
			next += (len + 2);
		}

		return devices;
	}

	bool AudioManager::IsInitComplete()
	{
		MutexLock lock(g_context_mutex);
		return g_context != NULL;
	}

	void AudioManager::OnPause()
	{
		for (auto i : g_sources)
		{
			ALint state;
			alGetSourcei(i, AL_SOURCE_STATE, &state);

			if (state == AL_PLAYING)
			{
				alSourcePause(i);
				g_sources_paused.AddLast(i);
			}
		}
	}

	void AudioManager::OnResume()
	{
		for (auto i : g_sources_paused)
		{
			alSourcePlay(i);
		}
		g_sources_paused.Clear();
	}

	void AudioManager::Init()
	{
		auto devices = get_devices();

		auto callback = [=](ALCdevice* device) {
			MutexLock lock(g_context_mutex);

			if (device != NULL)
			{
				g_device = device;

				auto context = alcCreateContext(device, NULL);
				if (context != NULL)
				{
					g_context = context;

					auto result = alcMakeContextCurrent(context);
					if (result == ALC_FALSE)
					{
						Log("alcMakeContextCurrent failed");
					}
				}
				else
				{
					Log("alcCreateContext failed");
				}
			}
			else
			{
				Log("alcOpenDeviceAsync get NULL device");
			}
		};

#ifdef VR_WINRT
		MutexLock lock(g_context_mutex);

		auto result = alcOpenDeviceAsync(
			devices[OAL_DEVICE_ID].CString(),
			callback
		);

		if (result == ALC_FALSE)
		{
			Log("alcOpenDeviceAsync failed");
		}
#else
		auto device = alcOpenDevice(devices[OAL_DEVICE_ID].CString());
		callback(device);
#endif
	}

	void AudioManager::Deinit()
	{
		if (g_context)
		{
			alcMakeContextCurrent(NULL);
			alcDestroyContext(g_context);
			g_context = NULL;
		}

		if (g_device)
		{
			alcCloseDevice(g_device);
			g_device = NULL;
		}

		g_sources.Clear();
		g_sources_paused.Clear();
	}

	void AudioManager::SetVolume(float volume)
	{
		alListenerf(AL_GAIN, volume);
	}

	void AudioManager::SetListener(AudioListener* listener)
	{
		auto pos = listener->GetTransform()->GetPosition();
		auto forward = listener->GetTransform()->GetForward();
		auto up = listener->GetTransform()->GetUp();

		float orientation[] =
		{
			forward.x, forward.y, forward.z,
			up.x, up.y, up.z,
		};
		float velocity[] = { 0, 0, 0 };

		alListenerfv(AL_POSITION, (float*) &pos);
		alListenerfv(AL_ORIENTATION, orientation);
		alListenerfv(AL_VELOCITY, velocity);
	}

	ALHandle AudioManager::CreateBuffer(int channel, int frequency, int bits, void* data, int size)
	{
		ALenum format = 0;

		switch (channel)
		{
			case 1:
				if (bits == 8)
					format = AL_FORMAT_MONO8;
				else
					format = AL_FORMAT_MONO16;
				break;
			case 2:
				if (bits == 8)
					format = AL_FORMAT_STEREO8;
				else
					format = AL_FORMAT_STEREO16;
				break;
		}

		ALuint buffer = 0;
		alGenBuffers(1, &buffer);
		if (buffer > 0)
		{
			alBufferData(buffer, format, data, size, frequency);
		}
		else
		{
			Log("alGenBuffers failed");
		}

		return buffer;
	}

	ALHandle AudioManager::CreateClipBuffer(AudioClip* clip, void* data)
	{
		return CreateBuffer(clip->GetChannels(), clip->GetFrequency(), clip->GetBits(), data, clip->GetBufferSize());
	}

	void AudioManager::DeleteClipBuffer(AudioClip* clip)
	{
		ALuint buffer = (ALuint) clip->GetBuffer();
		alDeleteBuffers(1, &buffer);
	}

	ALHandle AudioManager::CreateSource(AudioSource* source)
	{
		ALuint src = 0;
		alGenSources(1, &src);

		SetSourcePosition(source);
		SetSourceLoop(source);
		SetSourceVolume(source);
		if (source->GetClip())
		{
			SetSourceBuffer(source);
		}

		g_sources.AddLast(src);

		return src;
	}

	void AudioManager::DeleteSource(AudioSource* source)
	{
		ALuint src = (ALuint) source->GetSource();
		alDeleteSources(1, &src);

		g_sources.Remove(src);
	}

	void AudioManager::SetSourcePosition(AudioSource* source)
	{
		ALuint src = (ALuint) source->GetSource();
		alSourcefv(src, AL_POSITION, (ALfloat*) &source->GetTransform()->GetPosition());
	}

	void AudioManager::SetSourceLoop(AudioSource* source)
	{
		ALuint src = (ALuint) source->GetSource();
		alSourcei(src, AL_LOOPING, source->IsLoop());
	}

	void AudioManager::SetSourceBuffer(AudioSource* source)
	{
		ALuint src = (ALuint) source->GetSource();
		alSourcei(src, AL_BUFFER, (ALuint) source->GetClip()->GetBuffer());
	}

	void AudioManager::SetSourceQueueBuffer(AudioSource* source, ALHandle buffer)
	{
		ALuint src = (ALuint) source->GetSource();
		ALuint b = (ALuint) buffer;

		if (source->IsLoop())
		{
			ALint loop;
			alGetSourcei(src, AL_LOOPING, &loop);
			if (loop)
			{
				alSourcei(src, AL_LOOPING, AL_FALSE);
			}
		}

		alSourceQueueBuffers(src, 1, &b);
	}

	int AudioManager::GetSourceBufferQueued(AudioSource* source)
	{
		ALuint src = (ALuint) source->GetSource();

		int queued;
		alGetSourceiv(src, AL_BUFFERS_QUEUED, &queued);

		return queued;
	}

	void AudioManager::ProcessSourceBufferQueue(AudioSource* source)
	{
		ALuint src = (ALuint) source->GetSource();

		int processed;
		alGetSourceiv(src, AL_BUFFERS_PROCESSED, &processed);
		if (processed > 0)
		{
			Vector<ALuint> buffers(processed);
			alSourceUnqueueBuffers(src, processed, &buffers[0]);
			alDeleteBuffers(processed, &buffers[0]);
		}
	}

	void AudioManager::DeleteSourceBufferQueue(AudioSource* source)
	{
		ALuint src = (ALuint) source->GetSource();

		int queued;
		alGetSourceiv(src, AL_BUFFERS_QUEUED, &queued);
		if (queued > 0)
		{
			Vector<ALuint> buffers(queued);
			alSourceUnqueueBuffers(src, queued, &buffers[0]);
			alDeleteBuffers(queued, &buffers[0]);
		}
	}

	void AudioManager::SetSourceVolume(AudioSource* source)
	{
		ALuint src = (ALuint) source->GetSource();
		alSourcef(src, AL_GAIN, source->GetVolume());
	}

	void AudioManager::SetSourceOffset(AudioSource* source, float time)
	{
		ALuint src = (ALuint) source->GetSource();
		alSourcef(src, AL_SEC_OFFSET, time);
	}

	float AudioManager::GetSourceOffset(AudioSource* source)
	{
		float time;
		ALuint src = (ALuint) source->GetSource();
		alGetSourcef(src, AL_SEC_OFFSET, &time);

		return time;
	}

	void AudioManager::PlaySource(AudioSource* source)
	{
		ALuint src = (ALuint) source->GetSource();
		alSourcePlay(src);
	}

	void AudioManager::PauseSource(AudioSource* source)
	{
		ALuint src = (ALuint) source->GetSource();
		alSourcePause(src);
	}

	void AudioManager::StopSource(AudioSource* source)
	{
		ALuint src = (ALuint) source->GetSource();
		alSourceStop(src);
	}

	bool AudioManager::IsSourcePlaying(AudioSource* source)
	{
		ALuint src = (ALuint) source->GetSource();
		ALint state;
		alGetSourcei(src, AL_SOURCE_STATE, &state);

		return state == AL_PLAYING;
	}
}
