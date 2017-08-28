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

#include "Object.h"

namespace Viry3D
{
	Map<String, Ref<Object>> Object::m_cache;
	Mutex Object::m_mutex;

	void Object::Init()
	{
	}

	void Object::Deinit()
	{
		m_mutex.lock();
		m_cache.Clear();
		m_mutex.unlock();
	}

	Ref<Object> Object::GetCache(String path)
	{
		Ref<Object> obj;

		m_mutex.lock();

		Ref<Object>* ptr;
		if (m_cache.TryGet(path, &ptr))
		{
			obj = *ptr;
		}

		m_mutex.unlock();

		return obj;
	}

	void Object::AddCache(String path, const Ref<Object>& obj)
	{
		m_mutex.lock();
		m_cache.Add(path, obj);
		m_mutex.unlock();
	}

	Object::Object():
		m_name("Object")
	{
		static int s_id = 0;
		m_id = s_id++;
	}

	Object::~Object()
	{
	}

	void Object::DeepCopy(const Ref<Object>& source)
	{
		m_name = source->m_name;
	}
}
