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

#include "Application.h"
#include "World.h"
#include "GameObject.h"
#include "Resource.h"
#include "graphics/Camera.h"
#include "graphics/Graphics.h"
#include "memory/Memory.h"
#include "time/Time.h"
#include "ui/UICanvasRenderer.h"
#include "ui/UILabel.h"
#include "ui/Font.h"
#include "Input.h"
#include "Profiler.h"
#include "Debug.h"
#include <mutex>

#if VR_WINDOWS
#include <Windows.h>
#endif

#if VR_ANDROID
#include "android/jni.h"
#endif

namespace Viry3D
{
	Application* Application::m_instance;
	String Application::m_data_path;

	Application* Application::Current()
	{
		return m_instance;
	}

#if !VR_IOS
	String Application::DataPath()
	{
		static std::mutex s_mutex;

		s_mutex.lock();
		if (m_data_path.Empty())
		{
#if VR_WINDOWS
			char buffer[MAX_PATH];
			::GetModuleFileName(NULL, buffer, MAX_PATH);
			String path = buffer;
			path = path.Replace("\\", "/").Substring(0, path.LastIndexOf("\\")) + "/Assets";
			m_data_path = path;
#endif
		}
		s_mutex.unlock();

		return m_data_path;
	}

	String Application::SavePath()
	{
		static String s_path;

		if (s_path.Empty())
		{
#if VR_WINDOWS
			s_path = DataPath();
#endif

#if VR_ANDROID
			s_path = DataPath();
#endif
		}

		return s_path;
	}
#endif

	void Application::SetDataPath(String path)
	{
#if VR_ANDROID
		m_data_path = path;
#endif
	}

	Application::Application()
	{
		m_instance = this;
		m_start = false;
		m_quit = false;
		m_name = "Viry3D::Application";
		m_init_width = 1280;
		m_init_height = 720;
		m_init_fps = -1;
		m_pre_runloop = RefMake<RunLoop>();
		m_post_runloop = RefMake<RunLoop>();
		m_thread_pool_update = RefMake<ThreadPool>(4);
	}

	Application::~Application()
	{
		World::Deinit();
		Graphics::Deinit();

		m_instance = NULL;
	}

	void Application::SetInitSize(int width, int height)
	{
		m_init_width = width;
		m_init_height = height;
	}

	void Application::SetInitFPS(int fps)
	{
		m_init_fps = fps;
	}

	void Application::SetName(String name)
	{
		m_name = name;
	}

	String Application::GetName()
	{
		return m_name;
	}

	void Application::Run()
	{
		assert(!m_start);

		this->OnInit();

		while (!m_quit)
		{
			this->EnsureFPS();
			this->OnUpdate();
			this->OnDraw();
		}
	}

	void Application::OnInit()
	{
		m_start = true;
		Graphics::Init(m_init_width, m_init_height, m_init_fps);
		World::Init();
		this->Start();
	}

	void Application::AddAsyncUpdateTask(Thread::Task task)
	{
		m_thread_pool_update->AddTask(task);
	}

	void Application::EnsureFPS()
	{
		auto fps = Graphics::GetDisplay()->GetPreferredFPS();
		if (fps > 0)
		{
			static long long frame_time_start_last = 0;
			auto frame_time_min = 1000 / fps;
			auto frame_time_start_this = Time::GetTimeMS();
			auto frame_time_delta = frame_time_start_this - frame_time_start_last;
			if (frame_time_delta < frame_time_min)
			{
				Thread::Sleep((int) (frame_time_min - frame_time_delta));
			}
			frame_time_start_last = Time::GetTimeMS();
		}
	}

	void Application::OnUpdate()
	{
		this->UpdateFPSUI();
		Profiler::Reset();

		Profiler::SampleBegin("Application::OnUpdate");

		Time::Update();

		m_pre_runloop->Run();
		this->Update();
		World::Update();
		m_post_runloop->Run();
		m_thread_pool_update->Wait();

		if (Input::GetKeyDown(KeyCode::Backspace))
		{
			Quit();
		}

		Input::Update();

		Profiler::SampleEnd();
	}

	void Application::OnDraw()
	{
		Profiler::SampleBegin("Application::OnDraw");

		Graphics::Render();

		Profiler::SampleEnd();
	}

	void Application::OnPause()
	{
		Graphics::OnPause();
		World::OnPause();
	}

	void Application::OnResume()
	{
		Graphics::OnResume();
		World::OnResume();
	}

	void Application::OnResize(int width, int height)
	{
		Graphics::OnResize(width, height);

		this->OnResizeFPSUI(width, height);
	}

	void Application::Quit()
	{
		m_instance->m_quit = true;

#if VR_ANDROID
		java_quit_application();
#endif
	}

	void Application::RunTaskInPreLoop(RunLoop::Task task)
	{
		m_instance->m_pre_runloop->Add(task);
	}

	void Application::RunTaskInPostLoop(RunLoop::Task task)
	{
		m_instance->m_post_runloop->Add(task);
	}

	void Application::CreateFPSUI(int font_size, int camera_depth, int layer, Ref<FrameBuffer> render_target)
	{
		auto camera = GameObject::Create("camera")->AddComponent<Camera>();
		camera->SetClipNear(-1);
		camera->SetClipFar(1);
		camera->SetDepth(camera_depth);
		camera->SetClearFlags(CameraClearFlags::Nothing);
		camera->SetCullingMask(1 << layer);
		camera->SetFrameBuffer(render_target);
		camera->SetOrthographic(true);
		camera->SetOrthographicSize(camera->GetTargetHeight() / 2.0f);

		auto canvas = GameObject::Create("canvas")->AddComponent<UICanvasRenderer>();
		canvas->GetTransform()->SetParent(camera->GetTransform());
		canvas->SetSize(Vector2((float) camera->GetTargetWidth(), (float) camera->GetTargetHeight()));

		auto font = Resource::LoadFont("Assets/font/arial.ttf");

		auto fps = GameObject::Create("fps")->AddComponent<UILabel>();
		fps->GetTransform()->SetParent(canvas->GetTransform());
		fps->SetFont(font);
		fps->SetFontSize(font_size);
		fps->SetColor(Color(0, 1, 0, 1));
		fps->SetText("fps");
		fps->SetRich(true);
		fps->SetAlignment(TextAlignment::UpperLeft);

		fps->SetAnchors(Vector2(0, 1), Vector2(0, 1));
		fps->SetOffsets(Vector2(0, (float) -camera->GetTargetHeight()), Vector2((float) camera->GetTargetWidth(), 0));
		fps->OnAnchor();

		canvas->GetGameObject()->SetLayerRecursively(layer);

		m_fps = fps;
	}

	void Application::UpdateFPSUI()
	{
		if (!m_fps.expired())
		{
			auto text = String::Format("%s %dx%d\nfps:%d dc:%d",
				Graphics::GetDisplay()->GetDeviceName().CString(),
				Graphics::GetDisplay()->GetWidth(),
				Graphics::GetDisplay()->GetHeight(),
				Time::GetFPS(), Graphics::draw_call);
			auto samples = Profiler::GetSamples();
			for (auto& i : samples)
			{
				text += String::Format("\n%s: t:%.3f c:%d", i.first.CString(), i.second.time * 1000, i.second.call_count);
			}

			m_fps.lock()->SetText(text);
		}
	}

	void Application::OnResizeFPSUI(int width, int height)
	{
		if (!m_fps.expired())
		{
			auto fps = m_fps.lock();

			auto camera = fps->GetTransform()->GetParent().lock()->GetParent().lock()->GetGameObject()->GetComponent<Camera>();
			camera->SetOrthographicSize(camera->GetTargetHeight() / 2.0f);

			auto canvas = m_fps.lock()->GetTransform()->GetParent().lock()->GetGameObject()->GetComponent<UICanvasRenderer>();
			canvas->SetSize(Vector2((float) camera->GetTargetWidth(), (float) camera->GetTargetHeight()));

			fps->SetOffsets(Vector2(0, (float) -camera->GetTargetHeight()), Vector2((float) camera->GetTargetWidth(), 0));
			fps->OnAnchor();
		}
	}
}
