/*
* Viry3D
* Copyright 2014-2019 by Stack - stackos@qq.com
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

#include "memory/Ref.h"

namespace Viry3D
{
	typedef unsigned char byte;

	class ByteBuffer
	{
	public:
		ByteBuffer(int size = 0);
		ByteBuffer(const ByteBuffer& buffer);
		ByteBuffer(byte* bytes, int size);
		~ByteBuffer();

		byte* Bytes() const;
		int Size() const;

		ByteBuffer& operator =(const ByteBuffer& buffer);
		byte& operator [](int index);
		const byte& operator [](int index) const;

	private:
		void Free();

		int m_size;
		byte* m_bytes;
		Ref<bool> m_ref_count;
		bool m_weak_ref;
	};
}
