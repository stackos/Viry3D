/*
* Viry3D
* Copyright 2014-2018 by Stack - stackos@qq.com
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

#include "RunLoop.h"
#include "memory/Ref.h"
#include "string/String.h"
#include "thread/Thread.h"
#include <assert.h>

#define APP_VERSION			0x00010000
#define APP_VERSION_NAME	"1.0"

namespace Viry3D
{
	class UILabel;
	struct FrameBuffer;

	class Application
	{
	public:
		static Application* Current();
		static void Quit();
		static void RunTaskInPreLoop(const RunLoop::Task& task);
		static void RunTaskInPostLoop(const RunLoop::Task& task);
		static String DataPath();
		static String SavePath();
		static void SetDataPath(const String& path);

		virtual ~Application();

		void SetInitSize(int width, int height);
		void SetInitFPS(int fps);
		void SetName(const String& name);
		String GetName();

		void Run();
		void OnInit();
		void OnUpdate();
		void OnDraw();
		void AddAsyncUpdateTask(const Thread::Task& task);
		void EnsureFPS();
        bool IsPaused() const { return m_paused; }

		virtual void Start() { }
		virtual void Update() { }
		virtual void OnResize(int width, int height);
        virtual void OnPause();
        virtual void OnResume();
        
	protected:
		Application();

	private:
		static Application* m_instance;
		static String m_data_path;

		bool m_start;
		bool m_quit;
        bool m_paused;
		String m_name;
		int m_init_width;
		int m_init_height;
		int m_init_fps;
		Ref<RunLoop> m_pre_runloop;
		Ref<RunLoop> m_post_runloop;
		Ref<ThreadPool> m_thread_pool_update;
	};
}
