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

namespace Viry3D
{
	template<class V, size_t S>
	class Array
	{
	public:
		Array():m_size(S) { }

		int Size() const { return (int) m_size; }
		int SizeInBytes() const { return sizeof(V) * m_size; }

		V& operator [](int index) { return m_array[index]; }
		const V& operator [](int index) const { return m_array[index]; }

	private:
        V m_array[S];
        const size_t m_size;
	};
}
