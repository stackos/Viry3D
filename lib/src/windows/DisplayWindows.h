#pragma once

#include "graphics/DisplayBase.h"
#include <Windows.h>

#if VR_GLES
#include "gles/gles_include.h"
#endif

namespace Viry3D
{
	class DisplayWindows : public DisplayBase
	{
	public:
		DisplayWindows();
		virtual ~DisplayWindows();
		void Init(int width, int height, int fps);
		void Deinit() { }
		void ProcessSystemEvents();
		void KeepScreenOn(bool enable) { }


	private:
		void CreateSystemWindow();

	protected:
		HWND m_window;	
#if VR_GLES
		HDC m_hdc;
		HGLRC m_context;
		HGLRC m_shared_context;
#endif
	};
}