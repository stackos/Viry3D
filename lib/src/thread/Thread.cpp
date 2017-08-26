#include "Thread.h"
#include "Application.h"

namespace Viry3D
{
	void Thread::Sleep(int ms)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	}

	Thread::Thread(int id, ThreadInfo info)
	{
		m_id = id;
		m_close = false;
		m_info = info;
		m_thread = RefMake<std::thread>(&Thread::Run, this);
	}

	Thread::~Thread()
	{
		if(m_thread->joinable())
		{
			this->Wait();

			m_mutex.lock();
			m_close = true;
			m_condition.notify_one();
			m_mutex.unlock();
			m_thread->join();
		}
	}

	void Thread::Wait()
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		// wait until all job done
		m_condition.wait(lock, [this] ()
		{
			return m_job_queue.Empty();
		});
	}

	void Thread::AddTask(Task task)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_job_queue.AddLast(task);
		m_condition.notify_one();
	}

	void Thread::Run()
	{
		if(m_info.init)
		{
			m_info.init();
		}

		while(true)
		{
			Task task;
			{
				std::unique_lock<std::mutex> lock(m_mutex);

				// wait until has a job or close
				m_condition.wait(lock, [this]
				{
					return !m_job_queue.Empty() || m_close;
				});

				if(m_close)
				{
					break;
				}

				task = m_job_queue.First();
			}

			if(task.job)
			{
				auto any = task.job();

				if(task.done)
				{
					Application::RunTaskInPreLoop(
						RunLoop::Task(
							[any, task] ()
							{
								task.done(any);
							}
						)
					);
				}
			}

			{
				std::lock_guard<std::mutex> lock(m_mutex);
				m_job_queue.RemoveFirst();
				m_condition.notify_one();
			}
		}

		if(m_info.deinit)
		{
			m_info.deinit();
		}
	}

	int Thread::QueueLength()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_job_queue.Size();
	}

	ThreadPool::ThreadPool(int thread_count)
	{
		m_info.Resize(thread_count);
		m_threads.Resize(m_info.Size());
		for(int i = 0; i < m_threads.Size(); i++)
		{
			m_threads[i] = RefMake<Thread>(i, m_info[i]);
		}
	}

	ThreadPool::ThreadPool(const Vector<ThreadInfo>& info)
	{
		m_info = info;
		m_threads.Resize(m_info.Size());
		for(int i = 0; i < m_threads.Size(); i++)
		{
			m_threads[i] = RefMake<Thread>(i, m_info[i]);
		}
	}

	void ThreadPool::AddTask(Thread::Task task, int thread_index)
	{
		if(thread_index >= 0 && thread_index < m_threads.Size())
		{
			m_threads[thread_index]->AddTask(task);
		}
		else
		{
			int min_len = 0x7fffffff;
			int min_index = -1;

			for(int i = 0; i < m_threads.Size(); i++)
			{
				int len = m_threads[i]->QueueLength();
				if(min_len > len)
				{
					min_len = len;
					min_index = i;

					if(min_len == 0)
					{
						break;
					}
				}
			}

			m_threads[min_index]->AddTask(task);
		}
	}

	void ThreadPool::Wait()
	{
		for(auto& i : m_threads)
		{
			i->Wait();
		}
	}
}