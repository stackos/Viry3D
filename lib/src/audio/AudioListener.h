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

#pragma once

#include "Component.h"

namespace Viry3D
{
	class AudioListener: public Component
	{
		DECLARE_COM_CLASS(AudioListener, Component);

	public:
		static void Pause();
		static void Resume();
		static void SetVolume(float volume);
		static float GetVolume() { return m_volume; }

	protected:
		AudioListener() { }
		virtual void Start();

	private:
		static bool m_paused;
		static float m_volume;
	};
}
