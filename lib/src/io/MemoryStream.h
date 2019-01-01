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

#include "Stream.h"
#include "memory/ByteBuffer.h"
#include "memory/Memory.h"
#include "string/String.h"
#include "Debug.h"

namespace Viry3D
{
	class MemoryStream : public Stream
	{
	public:
		MemoryStream(const ByteBuffer& buffer);
		virtual int Read(void* buffer, int size);
		virtual int Write(void* buffer, int size);
		template<class T>
		T Read();
		template<class T>
		void Write(const T& t);
		String ReadString(int size);

	private:
		ByteBuffer m_buffer;
	};

	template<class T>
	T MemoryStream::Read()
	{
		T t;
		Read((void*) &t, sizeof(T));
		return t;
	}

	template<class T>
	void MemoryStream::Write(const T& t)
	{
		Write((void*) &t, sizeof(T));
	}
}
