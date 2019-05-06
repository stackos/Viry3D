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

#include "container/Vector.h"
#include "container/List.h"
#include "memory/Ref.h"
#include "Action.h"
#include <thread>
#include <mutex>
#include <condition_variable>

namespace Viry3D
{
	typedef std::mutex Mutex;

    class Object;

	class Thread
	{
	public:
		struct Task
		{
            typedef std::function<Ref<Object>()> Job;
            typedef std::function<void(const Ref<Object>&)> CompleteCallback;

			Job job;
            CompleteCallback complete;
		};

		static void Sleep(int ms);
        Thread(Action init, Action done);
		~Thread();
        void Wait();
        int GetQueueLength();
		void AddTask(const Task& task);

	private:
		void Run();

		Ref<std::thread> m_thread;
        List<Task> m_job_queue;
        Mutex m_mutex;
		std::condition_variable m_condition;
		bool m_close;
        Action m_init_action;
        Action m_done_action;
	};

	class ThreadPool
	{
	public:
		ThreadPool(int thread_count, Action init = nullptr, Action done = nullptr);
		void WaitAll();
		int GetThreadCount() const { return m_threads.Size(); }
        void AddTask(const Thread::Task& task, int thread_index = -1);

	private:
		Vector<Ref<Thread>> m_threads;
	};
}
