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

#pragma once

namespace Viry3D
{
	template<class V>
	struct FastListNode
	{
		V value;
		FastListNode* prev;
		FastListNode* next;
	};

	template<class V>
	class FastList
	{
	public:
		FastList();
		~FastList();

		void AddFirst(const V& v);
		void AddLast(const V& v);
		FastListNode<V>* Begin() { return m_first; }
		FastListNode<V>* End() { return m_last; }
		const FastListNode<V>* Begin() const { return m_first; }
		const FastListNode<V>* End() const { return m_last; }
		void RemoveFirst();
		void RemoveLast();
		void Clear();
		int Size() const { return m_size; }
		bool Empty() const { return m_size == 0; }
        FastListNode<V>* AddBefore(FastListNode<V>* node, const V& v);
        FastListNode<V>* AddAfter(FastListNode<V>* node, const V& v);
		bool Remove(const V& v);
		void RemoveAll(const V& v);
		FastListNode<V>* Remove(FastListNode<V>* n);

	private:
		FastListNode<V>* m_first;
		FastListNode<V>* m_last;
		int m_size;
	};

	template<class V>
	FastList<V>::FastList():
		m_size(0)
	{
		m_last = new FastListNode<V>();
		m_last->prev = NULL;
		m_last->next = NULL;
		m_first = m_last;
	}

	template<class V>
	FastList<V>::~FastList()
	{
		FastListNode<V>* p = m_first;
		FastListNode<V>* t;
		while (p != NULL)
		{
			t = p;
			p = p->next;
			delete t;
		}
	}

	template<class V>
	void FastList<V>::AddFirst(const V& v)
	{
		FastListNode<V>* n = new FastListNode<V>();
		n->value = v;
		n->prev = NULL;
		n->next = m_first;
		m_first->prev = n;
		m_first = n;
		m_size++;
	}

	template<class V>
	void FastList<V>::AddLast(const V& v)
	{
		FastListNode<V>* n = new FastListNode<V>();
		n->value = v;
		n->prev = m_last->prev;
		n->next = m_last;
		if (m_size > 0)
		{
			m_last->prev->next = n;
		}
		m_last->prev = n;
		if (m_size == 0)
		{
			m_first = n;
		}
		m_size++;
	}

	template<class V>
	void FastList<V>::RemoveFirst()
	{
		if (m_size > 0)
		{
			FastListNode<V>* t = m_first;
			m_first->next->prev = NULL;
			m_first = m_first->next;
			delete t;
			m_size--;
		}
	}

	template<class V>
	void FastList<V>::RemoveLast()
	{
		if (m_size > 0)
		{
			FastListNode<V>* t = m_last->prev;
			if (m_last->prev->prev != NULL)
			{
				m_last->prev->prev->next = m_last;
			}
			m_last->prev = m_last->prev->prev;
			if (m_size == 1)
			{
				m_first = m_last;
			}
			delete t;
			m_size--;
		}
	}

	template<class V>
	void FastList<V>::Clear()
	{
		FastListNode<V>* p = m_first;
		FastListNode<V>* t;
		while (p->next != NULL)
		{
			t = p;
			p = p->next;
			delete t;
		}
		m_first = m_last;
		m_size = 0;
	}

    template<class V>
    FastListNode<V>* FastList<V>::AddBefore(FastListNode<V>* node, const V& v)
    {
        FastListNode<V>* n = new FastListNode<V>();
        n->value = v;
        n->prev = node->prev;
        n->next = node;
        if (node->prev)
        {
            node->prev->next = n;
        }
        node->prev = n;
        m_size++;
        return n;
    }

    template<class V>
    FastListNode<V>* FastList<V>::AddAfter(FastListNode<V>* node, const V& v)
    {
        FastListNode<V>* n = new FastListNode<V>();
        n->value = v;
        n->prev = node;
        n->next = node->next;
        node->next->prev = n;
        node->next = n;
        m_size++;
        return n;
    }

	template<class V>
	bool FastList<V>::Remove(const V& v)
	{
		for (FastListNode<V>* i = m_first; i != m_last; i = i->next)
		{
			if (i->value == v)
			{
				if (i->prev != NULL)
				{
					i->prev->next = i->next;
				}
				i->next->prev = i->prev;
				if (m_first == i)
				{
					m_first = i->next;
				}
				delete i;
				m_size--;
				return true;
			}
		}

		return false;
	}

	template<class V>
	void FastList<V>::RemoveAll(const V& v)
	{
		FastListNode<V>* t;
		for (FastListNode<V>* i = m_first; i != m_last; )
		{
			t = i;
			i = i->next;

			if (t->value == v)
			{
				if (t->prev != NULL)
				{
					t->prev->next = t->next;
				}
				t->next->prev = t->prev;
				if (m_first == t)
				{
					m_first = t->next;
				}
				delete t;
				m_size--;
			}
		}
	}

	template<class V>
	FastListNode<V>* FastList<V>::Remove(FastListNode<V>* n)
	{
		FastListNode<V>* next = n->next;

		if (n->prev != NULL)
		{
			n->prev->next = n->next;
		}
		n->next->prev = n->prev;
		if (m_first == n)
		{
			m_first = n->next;
		}
		delete n;
		m_size--;

		return next;
	}
}
