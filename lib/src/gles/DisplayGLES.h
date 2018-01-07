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

#if VR_IOS
#include "ios/DisplayIOS.h"
#elif VR_MAC
#include "mac/DisplayMac.h"
#elif VR_ANDROID
#include "android/DisplayAndroid.h"
#elif VR_WINDOWS
#include "windows/DisplayWindows.h"
#endif

#include "memory/Ref.h"
#include "graphics/IndexBuffer.h"
#include "string/String.h"
#include <mutex>

namespace Viry3D
{
	class VertexBuffer;
	class Shader;
	class Thread;

	class DisplayGLESPrivate;

#if VR_IOS
	class DisplayGLES: public DisplayIOS
#elif VR_MAC
    class DisplayGLES: public DisplayMac
#elif VR_ANDROID
	class DisplayGLES: public DisplayAndroid
#elif VR_WINDOWS
	class DisplayGLES: public DisplayWindows
#endif
    {
		friend class DisplayGLESPrivate;

	public:
		DisplayGLES();
		void Init(int width, int height, int fps);
		void Deinit();
		void OnResize(int width, int height);
		void OnPause();
		void OnResume();
		void BeginFrame() { }
		void EndFrame() { }
		void WaitQueueIdle() { }
		void BindVertexArray();
		void BindVertexBuffer(const VertexBuffer* buffer);
		void BindIndexBuffer(const IndexBuffer* buffer, IndexType index_type);
		void BindVertexAttribArray(const Ref<Shader>& shader, int pass_index);
		void DrawIndexed(int start, int count, IndexType index_type);
		void DisableVertexArray(const Ref<Shader>& shader, int pass_index);
		void SubmitQueue(void* cmd) { }
		virtual void BeginRecord(const String& file);
		virtual void EndRecord();

		int GetMinUniformBufferOffsetAlignment() const { return m_uniform_buffer_offset_alignment; }

#if VR_ANDROID
		void EGLInit(int& width, int& height);
		void EGLDeinit();
		void EGLPause();
		void EGLResume(int& width, int& height);
#endif

#if VR_ANDROID || VR_WINDOWS
		void CreateSharedContext();
		void DestroySharedContext();
#endif
        
#if VR_ANDROID || VR_WINDOWS || VR_MAC
		int GetDefualtDepthRenderBuffer();
#endif
        
		void FlushContext();
		void SwapBuffers();

		const String& GetDeviceName() const { return m_device_name; }

	private:
		void RecordBuffer();

	private:
		Ref<DisplayGLESPrivate> m_private;

#if VR_ANDROID
		EGLDisplay m_display;
		EGLSurface m_surface;
		EGLContext m_context;
		EGLConfig m_config;
#endif

#if VR_ANDROID || VR_WINDOWS || VR_MAC
		GLuint m_default_depth_render_buffer;
#endif

		int m_uniform_buffer_offset_alignment;
		String m_extensions;
		String m_device_name;
		GLuint m_default_vao;
	};
}
