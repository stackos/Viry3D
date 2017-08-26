#pragma once

#include "Component.h"
#include "AudioManager.h"

namespace Viry3D
{
	class AudioClip;

	class AudioSource : public Component
	{
		DECLARE_COM_CLASS(AudioSource, Component);

	public:
		virtual ~AudioSource();
		void SetClip(const String& file);
		void SetClip(const Ref<AudioClip>& clip);
		Ref<AudioClip> GetClip() const { return m_clip; }
		ALHandle GetSource() const { return m_source; }
		void SetLoop(bool loop);
		bool IsLoop() const { return m_loop; }
		void SetVolume(float volume);
		float GetVolume() const { return m_volume; }
		void SetTime(float time);
		float GetTime();
		void Play();
		void Pause();
		void Stop();
		bool IsPlaying();
		void PlayMp3File(const String& file);

	protected:
		AudioSource() :
			m_source(0),
			m_loop(false),
			m_volume(1.0f),
			m_mp3_buffer(NULL)
		{ }
		virtual void Awake();
		virtual void OnTranformChanged();

	private:
		Ref<AudioClip> m_clip;
		ALHandle m_source;
		bool m_loop;
		float m_volume;
		void* m_mp3_buffer;
		String m_mp3_file;
	};
}