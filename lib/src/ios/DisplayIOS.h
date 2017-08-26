#pragma once

#include "graphics/DisplayBase.h"

namespace Viry3D {

class DisplayIOS : public DisplayBase {
public:
	void Init(int width, int height, int fps);
    void Deinit();
    void* GetWindowBridge();
    void BindDefaultFramebuffer();
    int GetDefualtDepthRenderBuffer();
    void KeepScreenOn(bool enable);
    
    void CreateSharedContext();
    void DestroySharedContext();

private:
    void* m_window;
};

}
