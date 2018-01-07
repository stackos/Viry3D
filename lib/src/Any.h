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
	class Any
	{
	public:
		template<class V>
		Any(const V& v):
			m_value(new Type<V>(v))
		{
		}

		~Any()
		{
			delete m_value;
		}

		template<class V>
		const V& Get() const
		{
			return ((Type<V>*) m_value)->v;
		}

		template<class V>
		V& Get()
		{
			return ((Type<V>*) m_value)->v;
		}

		template<class V>
		void Set(const V& v)
		{
			((Type<V>*) m_value)->v = v;
		}

	private:
		struct Base
		{
			virtual ~Base()
			{
			}
		};

		template<class V>
		struct Type: public Base
		{
			Type(const V& v):
				v(v)
			{
			}
			virtual ~Type()
			{
			}

			V v;
		};

		Base* m_value;
	};
}
