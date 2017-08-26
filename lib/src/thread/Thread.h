#pragma once

#include "container/Vector.h"
#include "container/List.h"
#include "memory/Ref.h"
#include "Any.h"
#include <thread>
#include <mutex>
#include <condition_variable>

namespace Viry3D
{
	typedef std::function<void()> ThreadInit;
	typedef std::function<void()> ThreadDeinit;

	struct ThreadInfo
	{
		ThreadInit init;
		ThreadDeinit deinit;
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