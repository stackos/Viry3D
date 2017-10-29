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

#include "Any.h"
#include "Action.h"
#include "container/Vector.h"
#include "container/List.h"
#include "memory/Ref.h"
#include <thread>
#include <mutex>
#include <condition_variable>

namespace Viry3D
{
	typedef std::mutex Mutex;

	struct ThreadInfo
	{
		Action init;
		Action deinit;
	};

	class Thread
	{
	public:
		typedef std::function<Ref<Any>()> Job;
		typedef std::function<void(Ref<Any>)> DoneCallback;
		struct Task
		{
			Job job;
			DoneCallback done;
		};

		static void Sleep(int ms);
		Thread(int id, ThreadInfo info);
		~Thread();
		void AddTask(Task task);
		void Wait();
		int QueueLength();

	private:
		void Run();

		int m_id;
		Ref<std::thread> m_thread;
		List<Task> m_job_queue;
		std::mutex m_mutex;
		std::condition_variable m_condition;
		bool m_close;
		ThreadInfo m_info;
	};

	class ThreadPool
	{
	public:
		ThreadPool(int thread_count);
		ThreadPool(const Vector<ThreadInfo>& info);
		void Wait();
		int GetThreadCount() const { return m_threads.Size(); }

		//
		//	Task:
		//	job, run in thread pool
		//	done, run on main thread
		//
		void AddTask(Thread::Task task, int thread_index = -1);

	private:
		Vector<ThreadInfo> m_info;
		Vector<Ref<Thread>> m_threads;
	};
}
