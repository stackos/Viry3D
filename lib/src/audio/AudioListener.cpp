#include "AudioListener.h"
#include "AudioManager.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(AudioListener);

	bool AudioListener::m_paused = false;
	float AudioListener::m_volume = 1.0f;

	void AudioListener::Pause()
	{
		m_paused = true;
	}

	void AudioListener::Resume()
	{
		m_paused = false;
	}

	void AudioListener::SetVolume(float volume)
	{
		m_volume = volume;

		AudioManager::SetVolume(volume);
	}

	void AudioListener::DeepCopy(const Ref<Object>& source)
	{
		assert(!"can not copy this component");
	}

	void AudioListener::Start()
	{
		AudioManager::SetListener(this);
	}
}