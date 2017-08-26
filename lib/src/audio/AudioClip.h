#pragma once

#include "Object.h"
#include "AudioManager.h"

namespace Viry3D
{
	class AudioClip : public Object
	{
	public:
		static Ref<AudioClip> LoadFromFile(String file);

		virtual ~AudioClip();
		int GetChannels() const { return m_channels; }
		int GetBits() const { return m_bits; }
		int GetBufferSize() const { return m_size; }
		int GetFrequency() const { return m_frequency; }
		ALHandle GetBuffer() const { return m_buffer; }

	private:
		int m_channels;
		int m_frequency;
		int m_samples;
		float m_length;
		int m_size;
		int m_bits;
		ALHandle m_buffer;

		AudioClip() :
			m_channels(0),
			m_frequency(0),
			m_samples(0),
			m_length(0),
			m_size(0),
			m_bits(0),
			m_buffer(0)
		{ }
	};
}