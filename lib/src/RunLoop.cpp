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

#include "RunLoop.h"

namespace Viry3D
{
	RunLoop::RunLoop()
	{
		m_id_counter = InvalidFuncId;
	}

	RunLoop::~RunLoop()
	{
	}

	RunLoop::FuncId RunLoop::Add(Task task)
	{
		m_mutex.lock();

		FuncId id = ++m_id_counter;
		m_to_add.Add(id, task);

		m_mutex.unlock();

		return id;
	}

	void RunLoop::Remove(FuncId id)
	{
		m_to_remove.Add(id);
	}

	bool RunLoop::HasFunc(FuncId id) const
	{
		return m_items.Contains(id) || m_to_add.Contains(id);
	}

	void RunLoop::AddFuncs()
	{
		for (auto& i : m_to_add)
		{
			m_items.Add(i.first, i.second);
		}
		m_to_add.Clear();
	}

	void RunLoop::RemoveFuncs()
	{
		for (auto i : m_to_remove)
		{
			m_items.Remove(i);
			m_to_add.Remove(i);
		}
		m_to_remove.Clear();
	}

	void RunLoop::Run()
	{
		m_mutex.lock();

		this->RemoveFuncs();
		this->AddFuncs();

		for (auto& i : m_items)
		{
			i.second.func();

			if (i.second.once)
			{
				this->Remove(i.first);
			}
		}

		m_mutex.unlock();
	}
}
