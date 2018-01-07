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

#include "AudioClip.h"
#include "AudioManager.h"
#include "io/File.h"
#include "io/MemoryStream.h"

namespace Viry3D
{
	static bool is_wave(byte* bytes)
	{
		if (Memory::Compare(bytes, "RIFF", 4) == 0)
		{
			if (Memory::Compare(&bytes[8], "WAVE", 4) == 0)
			{
				if (Memory::Compare(&bytes[12], "fmt ", 4) == 0)
				{
					return true;
				}
			}
		}

		return false;
	}

	Ref<AudioClip> AudioClip::LoadFromFile(const String& file)
	{
		Ref<AudioClip> clip;

		if (File::Exist(file))
		{
			auto buffer = File::ReadAllBytes(file);

			if (is_wave(buffer.Bytes()))
			{
				auto c = std::shared_ptr<AudioClip>(new AudioClip());

				MemoryStream ms(buffer);

				int size = buffer.Size();
				int chunk_data_pos = 8;
				int chunk_size;
				bool data_found = false;
				short block_align;

				int pos = 0;
				ms.Read(NULL, 4);
				pos += 4;
				chunk_size = ms.Read<int>();
				pos += 4;
				chunk_size = 4;

				while (!data_found && pos < size)
				{
					// got to next chunk
					int cur_pos = pos;
					int offset = chunk_size - (cur_pos - chunk_data_pos);
					ms.Read(NULL, offset);
					pos += offset;

					char chunk_id[4];
					ms.Read(chunk_id, 4);
					pos += 4;

					chunk_size = ms.Read<int>();
					pos += 4;

					chunk_data_pos = pos;

					if (Memory::Compare(chunk_id, "fmt ", 4) == 0)
					{
						short fmt = ms.Read<short>();
						pos += 2;
						if (fmt != 1)
						{
							break;
						}

						short channels = ms.Read<short>();
						pos += 2;
						if (channels > 2)
						{
							break;
						}
						c->m_channels = channels;

						int sample_rate = ms.Read<int>();
						pos += 4;
						c->m_frequency = sample_rate;

						ms.Read(chunk_id, 4);
						pos += 4;

						block_align = ms.Read<short>();
						pos += 2;

						short bits = ms.Read<short>();
						pos += 2;
						c->m_bits = bits;
					}
					else if (Memory::Compare(chunk_id, "data", 4) == 0)
					{
						data_found = true;
						c->m_size = chunk_size;
					}
				}

				c->m_samples = c->m_size / block_align;
				c->m_length = c->m_samples / (float) c->m_frequency;

				if (data_found)
				{
					c->m_buffer = AudioManager::CreateClipBuffer(c.get(), &buffer.Bytes()[pos]);

					clip = c;
				}
			}
		}

		return clip;
	}

	AudioClip::~AudioClip()
	{
		if (m_buffer != 0)
		{
			AudioManager::DeleteClipBuffer(this);
			m_buffer = 0;
		}
	}
}
