#pragma once

#include "math/Vector3.h"

namespace Viry3D
{
	typedef unsigned int ALHandle;

	class AudioListener;
	class AudioClip;
	class AudioSource;

	class AudioManager
	{
	public:
		static void Init();
		static bool IsInitComplete();
		static void Deinit();
		static void OnPause();
		static void OnResume();
		static void SetVolume(float volume);
		static void SetListener(AudioListener* listener);
		static ALHandle CreateClipBuffer(AudioClip* clip, void* data);
		static void DeleteClipBuffer(AudioClip* clip);
		static ALHandle CreateBuffer(int channel, int frequency, int bits, void* data, int size);
		static ALHandle CreateSource(AudioSource* source);
		static void DeleteSource(AudioSource* source);
		static void SetSourcePosition(AudioSource* source);
		static void SetSourceLoop(AudioSource* source);
		static void SetSourceBuffer(AudioSource* source);
		static void SetSourceQueueBuffer(AudioSource* source, ALHandle buffer);
		static void ProcessSourceBufferQueue(AudioSource* source);
		static void DeleteSourceBufferQueue(AudioSource* source);
		static int GetSourceBufferQueued(AudioSource* source);
		static void SetSourceVolume(AudioSource* source);
		static void SetSourceOffset(AudioSource* source, float time);
		static float GetSourceOffset(AudioSource* source);
		static void PlaySource(AudioSource* source);
		static void PauseSource(AudioSource* source);
		static void StopSource(AudioSource* source);
		static bool IsSourcePlaying(AudioSource* source);
	};
}