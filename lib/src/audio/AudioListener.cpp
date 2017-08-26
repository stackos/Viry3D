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
