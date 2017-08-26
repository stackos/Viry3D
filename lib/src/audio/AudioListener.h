#pragma once

#include "Component.h"

namespace Viry3D
{
	class AudioListener : public Component
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