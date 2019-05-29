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
#include "private/backend/CommandStream.h"
#include "private/backend/CommandBufferQueue.h"
#include "Debug.h"
#include "Input.h"
#include "Scene.h"
#include "Resources.h"
#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/Camera.h"
#include "audio/AudioManager.h"
#include "time/Time.h"
#include <thread>

#if VR_WINDOWS
#include <Windows.h>
#elif VR_IOS
#import <UIKit/UIKit.h>
#elif VR_MAC
#import <Cocoa/Cocoa.h>
#elif VR_ANDROID
#include "android/jni.h"
#endif

using namespace filament;

namespace Viry3D
{
    void FreeBufferCallback(void* buffer, size_t size, void* user)
    {
		Memory::Free(buffer, (int) size);
    }
    
	class EnginePrivate
	{
	public:
		static constexpr size_t CONFIG_MIN_COMMAND_BUFFERS_SIZE			= 1 * 1024 * 1024;
		static constexpr size_t CONFIG_COMMAND_BUFFERS_SIZE				= 3 * CONFIG_MIN_COMMAND_BUFFERS_SIZE;

		Engine* m_engine;
		backend::Backend m_backend;
		backend::Platform* m_platform = nullptr;
		void* m_shared_gl_context = nullptr;
		std::thread m_driver_thread;
		utils::CountDownLatch m_driver_barrier;
		utils::CountDownLatch m_frame_barrier;
		backend::Driver* m_driver = nullptr;
		backend::CommandBufferQueue m_command_buffer_queue;
		backend::DriverApi m_command_stream;
		void* m_native_window;
		int m_width;
		int m_height;
		uint64_t m_window_flags;
		backend::SwapChainHandle m_swap_chain;
		uint32_t m_frame_id = 0;
		backend::RenderTargetHandle m_render_target;
        String m_data_path;
        String m_save_path;
        bool m_quit = false;
        Ref<Scene> m_scene;
        Ref<ThreadPool> m_thread_pool;
        List<Action> m_actions;
        Mutex m_mutex;
        
		backend::DriverApi& GetDriverApi() { return m_command_stream; }

		EnginePrivate(Engine* engine, void* native_window, int width, int height, uint64_t flags, void* shared_gl_context):
			m_engine(engine),
#if VR_WINDOWS
			m_backend(backend::Backend::D3D11),
#elif VR_UWP
			m_backend(backend::Backend::D3D11),
#elif VR_ANDROID
			m_backend(backend::Backend::VULKAN),
#elif VR_USE_METAL
            m_backend(backend::Backend::METAL),
#else
			m_backend(backend::Backend::OPENGL),
#endif
			m_shared_gl_context(shared_gl_context),
			m_driver_barrier(1),
			m_frame_barrier(1),
			m_command_buffer_queue(CONFIG_MIN_COMMAND_BUFFERS_SIZE, CONFIG_COMMAND_BUFFERS_SIZE),
			m_native_window(native_window),
			m_width(width),
			m_height(height),
			m_window_flags(flags)
		{

		}

		~EnginePrivate()
		{
			if (m_driver)
			{
				delete m_driver;
			}
			
			backend::DefaultPlatform::destroy((backend::DefaultPlatform**) &m_platform);
		}

		void Init()
		{
			m_command_stream = backend::CommandStream(*m_driver, m_command_buffer_queue.getCircularBuffer());
			m_swap_chain = this->GetDriverApi().createSwapChain(m_native_window, m_window_flags);
			m_render_target = this->GetDriverApi().createDefaultRenderTarget();

            this->GetDataPath();
            this->GetSavePath();
            
#if !VR_WASM
            m_thread_pool = RefMake<ThreadPool>(4);
#endif
            
            Shader::Init();
            Texture::Init();
            AudioManager::Init();
			Resources::Init();
		}

		void Shutdown()
		{
            m_scene.reset();
            
			Resources::Done();
            AudioManager::Done();
            Texture::Done();
            Shader::Done();
            
            m_thread_pool.reset();
            
			this->GetDriverApi().destroyRenderTarget(m_render_target);

			if (!UTILS_HAS_THREADING)
			{
				this->Execute();
			}

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
		}

		void Loop()
		{
			m_platform = backend::DefaultPlatform::create(&m_backend);
			m_driver = m_platform->createDriver(m_shared_gl_context);
			m_driver_barrier.latch();
			if (!m_driver)
			{
				return;
			}

			while (true)
			{
				if (!this->Execute())
				{
					break;
				}
			}

			this->GetDriverApi().terminate();

			Log("driver thread terminated");
		}

		bool Execute()
		{
			auto buffers = m_command_buffer_queue.waitForCommands();
			if (buffers.empty())
			{
				return false;
			}

			for (int i = 0; i < buffers.size(); ++i)
			{
				auto& item = buffers[i];
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
			m_driver->purge();
			m_command_buffer_queue.flush();
		}

		void BeginFrame()
		{
            Time::Update();
            this->ProcessActions();
            
			++m_frame_id;

			auto& driver = this->GetDriverApi();
			driver.updateStreams(&driver);
			driver.makeCurrent(m_swap_chain, m_swap_chain);

			int64_t monotonic_clock_ns(std::chrono::steady_clock::now().time_since_epoch().count());
			driver.beginFrame(monotonic_clock_ns, m_frame_id);
		}

		void Render()
		{
			Camera::RenderAll();
			this->Flush();
		}

		void EndFrame()
		{
			this->GetDriverApi().commit(m_swap_chain);
			this->GetDriverApi().endFrame(m_frame_id);
			if (UTILS_HAS_THREADING)
			{
				this->GetDriverApi().queueCommand([this]() {
					m_frame_barrier.latch();
				});
			}
			this->Flush();

			if (UTILS_HAS_THREADING)
			{
				m_frame_barrier.await();
				m_frame_barrier.reset(1);
			}
            
#if VR_ANDROID
            if (Input::GetKeyDown(KeyCode::Backspace))
#else
            if (Input::GetKeyDown(KeyCode::Escape))
#endif
            {
                this->Quit();
            }
            
            Input::Update();
		}
        
        void Quit()
        {
            m_quit = true;
            
#if VR_ANDROID
            java_quit_application();
#endif
        }

        void PostAction(Action action)
        {
            m_mutex.lock();
            m_actions.AddLast(action);
            m_mutex.unlock();
        }
        
        void ProcessActions()
        {
            m_mutex.lock();
            for (const auto& action : m_actions)
            {
                if (action)
                {
                    action();
                }
            }
            m_actions.Clear();
            m_mutex.unlock();
        }
        
#if VR_WINDOWS
        const String& GetDataPath()
        {
            if (m_data_path.Empty())
            {
                char buffer[MAX_PATH];
                ::GetModuleFileName(nullptr, buffer, MAX_PATH);
                String path = buffer;
                path = path.Replace("\\", "/").Substring(0, path.LastIndexOf("\\")) + "/Assets";
                m_data_path = path;
            }
            
            return m_data_path;
        }
        
        const String& GetSavePath()
        {
            if (m_save_path.Empty())
            {
                m_save_path = this->GetDataPath();
            }
            
            return m_save_path;
        }
#elif VR_IOS
        const String& GetDataPath()
        {
            if (m_data_path.Empty())
            {
                String path = [[[NSBundle mainBundle] bundlePath] UTF8String];
                path += "/Assets";
                m_data_path = path;
            }
            
            return m_data_path;
        }
        
        const String& GetSavePath()
        {
            if (m_save_path.Empty())
            {
                NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
                NSString* doc_path = paths[0];
                m_save_path = [doc_path UTF8String];
            }
            
            return m_save_path;
        }
#elif VR_MAC
        const String& GetDataPath()
        {
            if (m_data_path.Empty())
            {
                String path = [[[NSBundle mainBundle] resourcePath] UTF8String];
                path += "/Assets";
                m_data_path = path;
            }
            
            return m_data_path;
        }
        
        const String& GetSavePath()
        {
            if (m_save_path.Empty())
            {
                NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
                NSString* doc_path = [paths objectAtIndex:0];
                m_save_path = [doc_path UTF8String];
            }
            
            return m_save_path;
        }
#elif VR_ANDROID
        const String& GetDataPath()
        {
            return m_data_path;
        }
        
        const String& GetSavePath()
        {
            return m_save_path;
        }
        
        void SetDataPath(const String& path)
        {
            m_data_path = path;
        }
        
        void SetSavePath(const String& path)
        {
            m_save_path = path;
        }
#elif VR_WASM
        const String& GetDataPath()
        {
            if (m_data_path.Empty())
            {
                m_data_path = "Assets";
            }
            return m_data_path;
        }
        
        const String& GetSavePath()
        {
            Log("web has no save path");
            
            return m_save_path;
        }
#elif VR_UWP
        const String& GetDataPath()
        {
            return m_data_path;
        }
        
        const String& GetSavePath()
        {
            return m_save_path;
        }
        
        void SetDataPath(const String& path)
        {
            m_data_path = path;
        }
        
        void SetSavePath(const String& path)
        {
            m_save_path = path;
        }
#endif

		void OnResize(void* native_window, int width, int height, uint64_t flags)
		{
			this->GetDriverApi().destroyRenderTarget(m_render_target);
			this->GetDriverApi().destroySwapChain(m_swap_chain);

			m_native_window = native_window;
			m_width = width;
			m_height = height;
			m_window_flags = flags;

			m_swap_chain = this->GetDriverApi().createSwapChain(m_native_window, m_window_flags);
			m_render_target = this->GetDriverApi().createDefaultRenderTarget();

            Camera::OnResizeAll(m_width, m_height);
		}
	};

	Engine* Engine::m_instance = nullptr;

	Engine* Engine::Create(void* native_window, int width, int height, uint64_t flags, void* shared_gl_context)
	{
		Engine* instance = Memory::New<Engine>(native_window, width, height, flags, shared_gl_context);
		m_instance = instance;

		if (!UTILS_HAS_THREADING)
		{
			instance->m_private->m_platform = backend::DefaultPlatform::create(&instance->m_private->m_backend);
			instance->m_private->m_driver = instance->m_private->m_platform->createDriver(instance->m_private->m_shared_gl_context);
			if (!instance->m_private->m_driver)
			{
				delete instance;
				m_instance = nullptr;
				return nullptr;
			}
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
				m_instance = nullptr;
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
				Memory::SafeDelete<Engine>(*engine);
				m_instance = nullptr;
			}
		}
	}

	Engine* Engine::Instance()
	{
		return m_instance;
	}

	Engine::Engine(void* native_window, int width, int height, uint64_t flags, void* shared_gl_context):
		m_private(Memory::New<EnginePrivate>(this, native_window, width, height, flags, shared_gl_context))
	{
	
	}
	
	Engine::~Engine()
	{
		Memory::SafeDelete<EnginePrivate>(m_private);
	}

	void Engine::Execute()
	{
        if (!m_private->m_scene)
        {
            m_private->m_scene = RefMake<Scene>();
        }
        m_private->m_scene->Update();
        
		m_private->BeginFrame();
		m_private->Render();
		m_private->EndFrame();

		if (!UTILS_HAS_THREADING)
		{
			m_private->Flush();
			m_private->Execute();
		}
	}

	backend::DriverApi& Engine::GetDriverApi()
	{
		return m_private->GetDriverApi();
	}

	const backend::Backend& Engine::GetBackend() const
	{
		return m_private->m_backend;
	}

	void* Engine::GetDefaultRenderTarget()
	{
		return &m_private->m_render_target;
	}

	const String& Engine::GetDataPath()
	{
		return m_private->GetDataPath();
	}

	const String& Engine::GetSavePath()
	{
		return m_private->GetSavePath();
	}

#if VR_ANDROID || VR_UWP
	void Engine::SetDataPath(const String& path)
	{
		m_private->SetDataPath(path);
	}

	void Engine::SetSavePath(const String& path)
	{
		m_private->SetSavePath(path);
	}
#endif

	void Engine::OnResize(void* native_window, int width, int height, uint64_t flags)
	{
		m_private->OnResize(native_window, width, height, flags);
	}

	int Engine::GetWidth() const
	{
		return m_private->m_width;
	}

	int Engine::GetHeight() const
	{
		return m_private->m_height;
	}

    bool Engine::HasQuit() const
    {
        return m_private->m_quit;
    }

    ThreadPool* Engine::GetThreadPool() const
    {
        return m_private->m_thread_pool.get();
    }
    
    void Engine::PostAction(Action action)
    {
        m_private->PostAction(action);
    }
}
