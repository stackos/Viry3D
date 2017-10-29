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

#include "container/Map.h"
#include "container/Vector.h"
#include "thread/Thread.h"
#include "Action.h"

namespace Viry3D
{
	class RunLoop
	{
	public:
		typedef int FuncId;
		static const FuncId InvalidFuncId = 0;
		struct Task
		{
			Action func;
			bool once;

			Task(Action func, bool once = true):
				func(func),
				once(once)
			{
			}
		};

		RunLoop();
		~RunLoop();

		/// run one frame
		void Run();

		/// add a func to the run loop
		FuncId Add(const Task& task);

	private:
		/// remove a func
		void Remove(FuncId id);
		/// test if a func has been attached
		bool HasFunc(FuncId id) const;
		/// add new funcs that have been added
		void AddFuncs();
		/// remove funcs that have been removed
		void RemoveFuncs();

		FuncId m_id_counter;
		Map<FuncId, Task> m_items;
		Map<FuncId, Task> m_to_add;
		Vector<FuncId> m_to_remove;
		Mutex m_mutex;
	};
}
