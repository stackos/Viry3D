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

#include <map>

namespace Viry3D
{
	template<class K, class V>
	class Map
	{
	public:
		Map() { }

		bool Add(const K& k, const V& v);
		bool Contains(const K& k) const;
		bool TryGet(const K& k, V** v);
		bool TryGet(const K& k, const V** v) const;
		bool Remove(const K& k);
		void Clear();
		int Size() const;
		bool Empty() const;

		V& operator [](const K& k);
		const V& operator [](const K& k) const;

		typedef typename std::map<K, V>::iterator Iterator;
		typedef typename std::map<K, V>::const_iterator ConstIterator;

		void AddRange(ConstIterator begin, ConstIterator end);
		Iterator Remove(ConstIterator pos);

		Iterator begin() { return m_map.begin(); }
		Iterator end() { return m_map.end(); }
		ConstIterator begin() const { return m_map.begin(); }
		ConstIterator end() const { return m_map.end(); }

	private:

		std::map<K, V> m_map;
	};

	template<class K, class V>
	bool Map<K, V>::Add(const K& k, const V& v)
	{
		std::pair<typename std::map<K, V>::iterator, bool> ret;
		ret = m_map.insert(std::pair<K, V>(k, v));
		return ret.second;
	}

	template<class K, class V>
	bool Map<K, V>::Contains(const K& k) const
	{
		return m_map.count(k) > 0;
	}

	template<class K, class V>
	bool Map<K, V>::Remove(const K& k)
	{
		return m_map.erase(k) == 1;
	}

	template<class K, class V>
	void Map<K, V>::Clear()
	{
		m_map.clear();
	}

	template<class K, class V>
	int Map<K, V>::Size() const
	{
		return (int) m_map.size();
	}

	template<class K, class V>
	bool Map<K, V>::Empty() const
	{
		return m_map.empty();
	}

	template<class K, class V>
	V& Map<K, V>::operator [](const K& k)
	{
		return m_map.at(k);
	}

	template<class K, class V>
	const V& Map<K, V>::operator [](const K& k) const
	{
		return m_map.at(k);
	}

	template<class K, class V>
	bool Map<K, V>::TryGet(const K& k, V** v)
	{
		Iterator find = m_map.find(k);
		if (find != m_map.end())
		{
			*v = &find->second;
			return true;
		}

		*v = nullptr;
		return false;
	}

	template<class K, class V>
	bool Map<K, V>::TryGet(const K& k, const V** v) const
	{
		ConstIterator find = m_map.find(k);
		if (find != m_map.end())
		{
			*v = &find->second;
			return true;
		}

		*v = nullptr;
		return false;
	}

	template<class K, class V>
	void Map<K, V>::AddRange(ConstIterator begin, ConstIterator end)
	{
		m_map.insert(begin, end);
	}

	template<class K, class V>
	typename Map<K, V>::Iterator Map<K, V>::Remove(ConstIterator pos)
	{
		return m_map.erase(pos);
	}
}
