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
	template<class V>
	class FastList
	{
    private:
        struct Node
        {
            V value;
            Node* prev;
            Node* next;
        };

    public:
        class ConstIterator;

        class Iterator
        {
            friend class FastList;
            Node* node;

        public:
            Iterator(Node* node):
                node(node)
            {
            }

            Iterator Prev() const
            {
                return Iterator(node->prev);
            }

            Iterator Next() const
            {
                return Iterator(node->next);
            }

            V& operator *() const
            {
                return node->value;
            }

            bool operator ==(const Iterator& i) const
            {
                return node == i.node;
            }

            bool operator !=(const Iterator& i) const
            {
                return node != i.node;
            }

            bool operator ==(const ConstIterator& i) const
            {
                return node == i.node;
            }

            bool operator !=(const ConstIterator& i) const
            {
                return node != i.node;
            }

            Iterator& operator ++()
            {
                node = node->next;
                return *this;
            }

            Iterator& operator --()
            {
                node = node->prev;
                return *this;
            }
        };

        class ConstIterator
        {
            friend class FastList;
            const Node* node;

        public:
            ConstIterator(const Node* node):
                node(node)
            {
            }

            ConstIterator Prev() const
            {
                return ConstIterator(node->prev);
            }

            ConstIterator Next() const
            {
                return ConstIterator(node->next);
            }

            const V& operator *() const
            {
                return node->value;
            }

            bool operator ==(const ConstIterator& i) const
            {
                return node == i.node;
            }

            bool operator !=(const ConstIterator& i) const
            {
                return node != i.node;
            }

            bool operator ==(const Iterator& i) const
            {
                return node == i.node;
            }

            bool operator !=(const Iterator& i) const
            {
                return node != i.node;
            }

            ConstIterator& operator ++()
            {
                node = node->next;
                return *this;
            }

            ConstIterator& operator --()
            {
                node = node->prev;
                return *this;
            }
        };

	public:
		FastList():
            m_last(new Node()),
            m_first(m_last),
            m_size(0),
            m_last_iter(m_last),
            m_last_const_iter(m_last)
        {
            m_last->prev = nullptr;
            m_last->next = nullptr;
        }

		~FastList()
        {
            Node* p = m_first;
            Node* t;
            while (p != nullptr)
            {
                t = p;
                p = p->next;
                delete t;
            }
        }

		void AddFirst(const V& v)
        {
            Node* n = new Node();
            n->value = v;
            n->prev = nullptr;
            n->next = m_first;
            m_first->prev = n;
            m_first = n;
            m_size++;
        }

		void AddLast(const V& v)
        {
            Node* n = new Node();
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

		void RemoveFirst()
        {
            if (m_size > 0)
            {
                Node* t = m_first;
                m_first->next->prev = nullptr;
                m_first = m_first->next;
                delete t;
                m_size--;
            }
        }

		void RemoveLast()
        {
            if (m_size > 0)
            {
                Node* t = m_last->prev;
                if (m_last->prev->prev != nullptr)
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
        
		void Clear()
        {
            Node* p = m_first;
            Node* t;
            while (p->next != nullptr)
            {
                t = p;
                p = p->next;
                delete t;
            }
            m_first = m_last;
            m_size = 0;
        }

		bool Remove(const V& v)
        {
            for (Node* i = m_first; i != m_last; i = i->next)
            {
                if (i->value == v)
                {
                    if (i->prev != nullptr)
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

		void RemoveAll(const V& v)
        {
            Node* t;
            for (Node* i = m_first; i != m_last; )
            {
                t = i;
                i = i->next;

                if (t->value == v)
                {
                    if (t->prev != nullptr)
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

        Iterator AddBefore(const Iterator& i, const V& v)
        {
            Node* node = i.node;
            Node* n = new Node();
            n->value = v;
            n->prev = node->prev;
            n->next = node;
            if (node->prev)
            {
                node->prev->next = n;
            }
            node->prev = n;
            m_size++;
            return Iterator(n);
        }

        Iterator AddAfter(const Iterator& i, const V& v)
        {
            Node* node = i.node;
            Node* n = new Node();
            n->value = v;
            n->prev = node;
            n->next = node->next;
            node->next->prev = n;
            node->next = n;
            m_size++;
            return Iterator(n);
        }

        Iterator Remove(const Iterator& i)
        {
            Node* n = i.node;
            Node* next = n->next;

            if (n->prev != nullptr)
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

            return Iterator(next);
        }

        int Size() const { return m_size; }
        bool Empty() const { return m_size == 0; }

        Iterator begin() { return Iterator(m_first); }
        const Iterator& end() { return m_last_iter; }
        ConstIterator begin() const { return ConstIterator(m_first); }
        const ConstIterator& end() const { return m_last_const_iter; }

	private:
        Node* m_last;
        Node* m_first;
		int m_size;
        Iterator m_last_iter;
        ConstIterator m_last_const_iter;
	};
}
