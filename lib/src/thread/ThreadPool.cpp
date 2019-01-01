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

#include "ThreadPool.h"
#include "Object.h"
#include "Application.h"
#include "graphics/Display.h"

namespace Viry3D
{
	void Thread::Sleep(int ms)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	}

	Thread::Thread(Action init, Action done):
        m_init_action(init),
        m_done_action(done)
	{
		m_close = false;
        m_thread = RefMake<std::thread>(&Thread::Run, this);
	}

	Thread::~Thread()
	{
		if (m_thread->joinable())
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
		std::unique_lock<Mutex> lock(m_mutex);

		// wait until all job done
		m_condition.wait(lock, [this]() {
			return m_job_queue.Empty();
		});
	}

	int Thread::GetQueueLength()
	{
		std::lock_guard<Mutex> lock(m_mutex);
		return m_job_queue.Size();
	}

    void Thread::AddTask(const Task& task)
    {
        std::lock_guard<Mutex> lock(m_mutex);
        m_job_queue.AddLast(task);
        m_condition.notify_one();
    }

    void Thread::Run()
    {
        bool gl_thread = m_init_action && m_done_action;
        
        if (m_init_action)
        {
            m_init_action();
        }

        while (true)
        {
            Task task;

            {
                std::unique_lock<Mutex> lock(m_mutex);

                // wait until has a job or close
                m_condition.wait(lock, [this] {
                    return !m_job_queue.Empty() || m_close;
                });

                if (m_close)
                {
                    break;
                }

                task = m_job_queue.First();
            }

            if (task.job)
            {
                Ref<Object> res = task.job();

#if VR_GLES
                if (gl_thread)
                {
                    Display::Instance()->Flush();
                }
#endif

                if (task.complete)
                {
                    Application::Instance()->PostAction([=]() {
                        task.complete(res);
                    });
                }
            }

            {
                std::lock_guard<Mutex> lock(m_mutex);
                m_job_queue.RemoveFirst();
                m_condition.notify_one();
            }
        }

        if (m_done_action)
        {
            m_done_action();
        }
    }

	ThreadPool::ThreadPool(int thread_count, Action init, Action done)
	{
		m_threads.Resize(thread_count);
		for (int i = 0; i < m_threads.Size(); ++i)
		{
			m_threads[i] = RefMake<Thread>(init, done);
		}
	}

	void ThreadPool::WaitAll()
	{
		for (auto& i : m_threads)
		{
			i->Wait();
		}
	}

    void ThreadPool::AddTask(const Thread::Task& task, int thread_index)
    {
        if (thread_index >= 0 && thread_index < m_threads.Size())
        {
            m_threads[thread_index]->AddTask(task);
        }
        else
        {
            int min_len = 0x7fffffff;
            int min_index = -1;

            for (int i = 0; i < m_threads.Size(); ++i)
            {
                int len = m_threads[i]->GetQueueLength();
                if (min_len > len)
                {
                    min_len = len;
                    min_index = i;

                    if (min_len == 0)
                    {
                        break;
                    }
                }
            }

            m_threads[min_index]->AddTask(task);
        }
    }
}
