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

#pragma once

#include "memory/ByteBuffer.h"
#include <array>

namespace Viry3D
{
	template<class V, size_t S>
	class Array
	{
	public:
		Array() { }

		int Size() const { m_array.size(); }
		byte* Bytes(int index = 0) const { return (byte*) &m_array[index]; }
		int SizeInBytes() const { return sizeof(V) * Size(); }

		V& operator [](int index) { return m_array[index]; }
		const V& operator [](int index) const { return m_array[index]; }

		typedef typename std::array<V, S>::iterator Iterator;
		typedef typename std::array<V, S>::const_iterator ConstIterator;

		Iterator begin() { return m_array.begin(); }
		Iterator end() { return m_array.end(); }
		ConstIterator begin() const { return m_array.begin(); }
		ConstIterator end() const { return m_array.end(); }

	private:

		std::array<V, S> m_array;
	};
}
