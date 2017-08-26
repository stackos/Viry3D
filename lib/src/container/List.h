#pragma once

#include <list>
#include <functional>

namespace Viry3D
{
	template<class V>
	class List
	{
	public:
		List() { }

		void AddFirst(const V& v);
		void AddLast(const V& v);
		V& First();
		const V& First() const;
		V& Last();
		const V& Last() const;
		void RemoveFirst();
		void RemoveLast();
		void Clear();
		int Size() const;
		bool Empty() const;
		bool Remove(const V& v);
		void RemoveAll(const V& v);

		typedef std::function<bool(const V&, const V&)> SortFunc;
		void Sort(SortFunc func);
		void Sort();

		typedef typename std::list<V>::iterator Iterator;
		typedef typename std::list<V>::const_iterator ConstIterator;

		Iterator AddBefore(ConstIterator pos, const V& v);
		Iterator AddAfter(ConstIterator pos, const V& v);
		Iterator AddRangeBefore(ConstIterator pos, ConstIterator begin, ConstIterator end);
		Iterator Remove(ConstIterator pos);

		Iterator begin() { return m_list.begin(); }
		Iterator end() { return m_list.end(); }
		ConstIterator begin() const { return m_list.begin(); }
		ConstIterator end() const { return m_list.end(); }

	private:
		std::list<V> m_list;
	};

	template<class V>
	void List<V>::Clear()
	{
		m_list.clear();
	}

	template<class V>
	int List<V>::Size() const
	{
		return (int) m_list.size();
	}

	template<class V>
	bool List<V>::Empty() const
	{
		return m_list.empty();
	}

	template<class V>
	void List<V>::AddFirst(const V& v)
	{
		m_list.push_front(v);
	}

	template<class V>
	void List<V>::AddLast(const V& v)
	{
		m_list.push_back(v);
	}

	template<class V>
	void List<V>::RemoveFirst()
	{
		m_list.pop_front();
	}

	template<class V>
	void List<V>::RemoveLast()
	{
		m_list.pop_back();
	}

	template<class V>
	V& List<V>::First()
	{
		return m_list.front();
	}

	template<class V>
	const V& List<V>::First() const
	{
		return m_list.front();
	}

	template<class V>
	V& List<V>::Last()
	{
		return m_list.back();
	}

	template<class V>
	const V& List<V>::Last() const
	{
		return m_list.back();
	}

	template<class V>
	typename List<V>::Iterator List<V>::AddBefore(ConstIterator pos, const V& v)
	{
		return m_list.insert(pos, v);
	}

	template<class V>
	typename List<V>::Iterator List<V>::AddAfter(ConstIterator pos, const V& v)
	{
		return m_list.insert(++pos, v);
	}

	template<class V>
	typename List<V>::Iterator List<V>::AddRangeBefore(ConstIterator pos, ConstIterator begin, ConstIterator end)
	{
		return m_list.insert(pos, begin, end);
	}

	template<class V>
	typename List<V>::Iterator List<V>::Remove(ConstIterator pos)
	{
		return m_list.erase(pos);
	}

	template<class V>
	bool List<V>::Remove(const V& v)
	{
		for(auto i = m_list.begin(); i != m_list.end(); i++)
		{
			if(*i == v)
			{
				m_list.erase(i);
				return true;
			}
		}

		return false;
	}

	template<class V>
	void List<V>::RemoveAll(const V& v)
	{
		m_list.remove(v);
	}

	template<class V>
	void List<V>::Sort(SortFunc func)
	{
		m_list.sort(func);
	}

	template<class V>
	void List<V>::Sort()
	{
		m_list.sort();
	}
}