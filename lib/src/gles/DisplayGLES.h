#pragma once

#if VR_IOS
#include "ios/DisplayIOS.h"
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
	{
#elif VR_ANDROID
	class DisplayGLES: public DisplayAndroid
	{
#elif VR_WINDOWS
	class DisplayGLES: public DisplayWindows
	{
#endif
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
		void BindVertexBuffer(const VertexBuffer* buffer);
		void BindIndexBuffer(const IndexBuffer* buffer, IndexType::Enum index_type);
		void BindVertexArray(const Ref<Shader>& shader, int pass_index);
		void DrawIndexed(int start, int count, IndexType::Enum index_type);
		void SubmitQueue(void* cmd) { }
		virtual void BeginRecord(const String& file);
		virtual void EndRecord();

		int GetMinUniformBufferOffsetAlignment() const { return m_uniform_buffer_offset_alignment; }

#if VR_ANDROID
		void EGLInit(int& width, int& height);
		void EGLDeinit();
		void EGLPause();
		void EGLResume();
#endif

#if VR_ANDROID || VR_WINDOWS
		void CreateSharedContext();
		void DestroySharedContext();
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

#if VR_ANDROID || VR_WINDOWS
		GLuint m_default_depth_render_buffer;
#endif

		int m_uniform_buffer_offset_alignment;
		String m_extensions;
		String m_device_name;
	};
}
