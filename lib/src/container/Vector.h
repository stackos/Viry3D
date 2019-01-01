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

#include "memory/ByteBuffer.h"
#include <vector>

namespace Viry3D
{
	template<class V>
	class Vector
	{
	public:
		Vector() { }
		Vector(int size);
        Vector(int size, const V& v);
        Vector(std::initializer_list<V> list);
        Vector(const Vector& from);
        Vector(Vector&& from);

		void Add(const V& v);
		void AddRange(const V* vs, int count);
        void AddRange(std::initializer_list<V> list);
        void AddRange(const Vector<V>& vs);
		void Clear();
		int Size() const;
		bool Empty() const;
		void Resize(int size);
		void Resize(int size, const V& v);
		byte* Bytes(int index = 0) const;
		int SizeInBytes() const;

		void Remove(const V& v);
		void Remove(int index);
		void RemoveRange(int index, int count);

		V& operator [](int index);
		const V& operator [](int index) const;
        Vector& operator =(const Vector& from);
        Vector& operator =(Vector&& from);

		typedef typename std::vector<V>::iterator Iterator;
		typedef typename std::vector<V>::const_iterator ConstIterator;

		Iterator begin() { return m_vector.begin(); }
		Iterator end() { return m_vector.end(); }
		ConstIterator begin() const { return m_vector.begin(); }
		ConstIterator end() const { return m_vector.end(); }

	private:
		std::vector<V> m_vector;
	};

	template<class V>
	Vector<V>::Vector(int size):
		m_vector(size)
	{
	}

    template<class V>
    Vector<V>::Vector(int size, const V& v):
        m_vector(size, v)
    {
    }

    template<class V>
    Vector<V>::Vector(std::initializer_list<V> list):
        m_vector(list)
    {
    }

    template<class V>
    Vector<V>::Vector(const Vector& from):
        m_vector(from.m_vector)
    {
    }

    template<class V>
    Vector<V>::Vector(Vector&& from):
        m_vector(std::move(from.m_vector))
    {
    }

	template<class V>
	void Vector<V>::Add(const V& v)
	{
		m_vector.push_back(v);
	}

	template<class V>
	void Vector<V>::AddRange(const V* vs, int count)
	{
		if (count > 0)
		{
			auto old_size = m_vector.size();
			m_vector.resize(old_size + count);

			for (int i = 0; i < count; ++i)
			{
				m_vector[old_size + i] = vs[i];
			}
		}
	}

    template<class V>
    void Vector<V>::AddRange(std::initializer_list<V> list)
    {
        m_vector.insert(m_vector.end(), list.begin(), list.end());
    }

    template<class V>
    void Vector<V>::AddRange(const Vector<V>& vs)
    {
        m_vector.insert(m_vector.end(), vs.begin(), vs.end());
    }

	template<class V>
	void Vector<V>::Clear()
	{
		m_vector.clear();
	}

	template<class V>
	int Vector<V>::Size() const
	{
		return (int) m_vector.size();
	}

	template<class V>
	bool Vector<V>::Empty() const
	{
		return m_vector.empty();
	}

	template<class V>
	byte* Vector<V>::Bytes(int index) const
	{
		return (byte*) &m_vector[index];
	}

	template<class V>
	int Vector<V>::SizeInBytes() const
	{
		return sizeof(V) * Size();
	}

	template<class V>
	void Vector<V>::Remove(const V& v)
	{
		for (int i = 0; i < this->Size(); ++i)
		{
			if (m_vector[i] == v)
			{
				this->Remove(i);
				break;
			}
		}
	}

	template<class V>
	void Vector<V>::Remove(int index)
	{
		m_vector.erase(m_vector.begin() + index);
	}

	template<class V>
	void Vector<V>::RemoveRange(int index, int count)
	{
		m_vector.erase(m_vector.begin() + index, m_vector.begin() + index + count);
	}

	template<class V>
	void Vector<V>::Resize(int size)
	{
		m_vector.resize(size);
	}

	template<class V>
	void Vector<V>::Resize(int size, const V& v)
	{
		m_vector.resize(size, v);
	}

	template<class V>
	V& Vector<V>::operator [](int index)
	{
		return m_vector[index];
	}

	template<class V>
	const V& Vector<V>::operator [](int index) const
	{
		return m_vector[index];
	}

    template<class V>
    Vector<V>& Vector<V>::operator =(const Vector<V>& from)
    {
        m_vector = from.m_vector;
        return *this;
    }

    template<class V>
    Vector<V>& Vector<V>::operator =(Vector<V>&& from)
    {
        m_vector = std::move(from.m_vector);
        return *this;
    }
}
