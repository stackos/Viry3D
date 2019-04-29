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

#include "Engine.h"
#include <backend/Platform.h>
#include <utils/compiler.h>
#include <utils/CountDownLatch.h>
#include <utils/JobSystem.h>
#include "private/backend/CommandStream.h"
#include "private/backend/CommandBufferQueue.h"
#include "private/backend/DriverApi.h"
#include <thread>

using namespace filament;

namespace Viry3D
{
	using LinearAllocatorArena = utils::Arena<
		utils::LinearAllocator,
		utils::LockingPolicy::NoLock>;
	using ArenaScope = utils::ArenaScope<LinearAllocatorArena>;

	class EnginePrivate
	{
	public:
		static constexpr size_t CONFIG_PER_RENDER_PASS_ARENA_SIZE		= 2 * 1024 * 1024;
		static constexpr size_t CONFIG_MIN_COMMAND_BUFFERS_SIZE			= 1 * 1024 * 1024;
		static constexpr size_t CONFIG_COMMAND_BUFFERS_SIZE				= 3 * CONFIG_MIN_COMMAND_BUFFERS_SIZE;

		Engine* m_engine;
		backend::Backend m_backend;
		backend::Platform* m_platform = nullptr;
		bool m_own_platform = false;
		void* m_shared_gl_context = nullptr;
		bool m_terminated = false;
		std::thread m_driver_thread;
		utils::CountDownLatch m_driver_barrier;
		backend::Driver* m_driver = nullptr;
		utils::JobSystem m_job_system;
		backend::CommandBufferQueue m_command_buffer_queue;
		backend::DriverApi m_command_stream;
		void* m_native_window;
		uint64_t m_window_flags;
		backend::Handle<backend::HwSwapChain> m_swap_chain;
		uint32_t m_frame_id = 0;
		LinearAllocatorArena m_per_render_pass_allocator;

		backend::Driver& GetDriver() const noexcept { return *m_driver; }
		backend::DriverApi& GetDriverApi() noexcept { return m_command_stream; }
		utils::JobSystem& GetJobSystem() noexcept { return m_job_system; }

		EnginePrivate(Engine* engine, void* native_window, uint64_t flags, void* shared_gl_context):
			m_engine(engine),
			m_backend(backend::Backend::DEFAULT),
			m_shared_gl_context(shared_gl_context),
			m_driver_barrier(1),
			m_command_buffer_queue(CONFIG_MIN_COMMAND_BUFFERS_SIZE, CONFIG_COMMAND_BUFFERS_SIZE),
			m_native_window(native_window),
			m_window_flags(flags),
			m_per_render_pass_allocator("per-renderpass allocator", CONFIG_PER_RENDER_PASS_ARENA_SIZE)
		{
			m_job_system.adopt();
		}

		~EnginePrivate()
		{
			if (m_driver)
			{
				delete m_driver;
			}
			
			if (m_own_platform)
			{
				backend::DefaultPlatform::destroy((backend::DefaultPlatform**) &m_platform);
			}
		}

		void Init()
		{
			m_command_stream = backend::CommandStream(*m_driver, m_command_buffer_queue.getCircularBuffer());
			
			m_swap_chain = this->GetDriverApi().createSwapChain(m_native_window, m_window_flags);
		}

		void Shutdown()
		{
			this->GetDriverApi().destroySwapChain(m_swap_chain);

			this->Flush();
			if (!UTILS_HAS_THREADING)
			{
				this->Execute();
			}
			m_command_buffer_queue.requestExit();

			if (UTILS_HAS_THREADING)
			{
				m_driver_thread.join();
			}

			m_job_system.emancipate();

			m_terminated = true;
		}

		void Loop()
		{
			m_platform = backend::DefaultPlatform::create(&m_backend);
			m_own_platform = true;
			m_driver = m_platform->createDriver(m_shared_gl_context);
			m_driver_barrier.latch();
			if (!m_driver)
			{
				return;
			}

			utils::JobSystem::setThreadName("EnginePrivate::Loop");
			utils::JobSystem::setThreadPriority(utils::JobSystem::Priority::DISPLAY);

			uint32_t id = std::thread::hardware_concurrency() - 1;

			while (true)
			{
				utils::JobSystem::setThreadAffinityById(id);
				if (!this->Execute())
				{
					break;
				}
			}

			this->GetDriverApi().terminate();
		}

		bool Execute()
		{
			auto buffers = m_command_buffer_queue.waitForCommands();
			if (buffers.empty())
			{
				return false;
			}

			for (auto& item : buffers)
			{
				if (item.begin)
				{
					m_command_stream.execute(item.begin);
					m_command_buffer_queue.releaseBuffer(item);
				}
			}

			return true;
		}

		void Flush()
		{
			this->GetDriver().purge();
			m_command_buffer_queue.flush();
		}

		void MakeCurrent()
		{
			this->GetDriverApi().makeCurrent(m_swap_chain, m_swap_chain);
		}

		void Commit()
		{
			this->GetDriverApi().commit(m_swap_chain);
		}

		bool BeginFrame()
		{
			++m_frame_id;

			auto& driver = this->GetDriverApi();
			driver.updateStreams(&driver);
			this->MakeCurrent();

			int64_t monotonic_clock_ns(std::chrono::steady_clock::now().time_since_epoch().count());
			driver.beginFrame(monotonic_clock_ns, m_frame_id);

			if (UTILS_HAS_THREADING)
			{
				
			}

			this->Prepare();

			return true;
		}

		void Prepare()
		{
			
		}

		void Render()
		{
			ArenaScope root_arena(m_per_render_pass_allocator);

			auto& js = this->GetJobSystem();

			auto master_job = js.setMasterJob(js.createJob());

			this->RenderJob(root_arena);

			this->Flush();

			js.runAndWait(master_job);
		}

		void RenderJob(ArenaScope& arena)
		{
			auto& js = this->GetJobSystem();
			auto& driver = this->GetDriverApi();
		}

		void EndFrame()
		{
			this->Commit();
			this->GetDriverApi().endFrame(m_frame_id);
			this->Flush();
		}
	};

	Engine* Engine::Create(void* native_window, uint64_t flags, void* shared_gl_context)
	{
		Engine* instance = new Engine(native_window, flags, shared_gl_context);

		if (!UTILS_HAS_THREADING)
		{
			instance->m_private->m_platform = backend::DefaultPlatform::create(&instance->m_private->m_backend);
			instance->m_private->m_own_platform = true;
			instance->m_private->m_driver = instance->m_private->m_platform->createDriver(instance->m_private->m_shared_gl_context);
			instance->m_private->Init();
			instance->m_private->Execute();
		}
		else
		{
			instance->m_private->m_driver_thread = std::thread(&EnginePrivate::Loop, instance->m_private);
			instance->m_private->m_driver_barrier.await();
			if (!instance->m_private->m_driver)
			{
				instance->m_private->m_driver_thread.join();
				delete instance;
				return nullptr;
			}
			instance->m_private->Init();
		}
		
		return instance;
	}

	void Engine::Destroy(Engine** engine)
	{
		if (engine)
		{
			if (*engine)
			{
				(*engine)->m_private->Shutdown();
				delete (*engine);
				*engine = nullptr;
			}
		}
	}

	Engine::Engine(void* native_window, uint64_t flags, void* shared_gl_context):
		m_private(new EnginePrivate(this, native_window, flags, shared_gl_context))
	{
	
	}
	
	Engine::~Engine()
	{
		delete m_private;
	}

	void Engine::Execute()
	{
		if (!UTILS_HAS_THREADING)
		{
			m_private->Flush();
			m_private->Execute();
		}

		if (m_private->BeginFrame())
		{
			m_private->Render();
			m_private->EndFrame();
		}
	}
}
