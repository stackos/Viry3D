#include "DisplayAndroid.h"
#include "jni.h"

namespace Viry3D
{
	void DisplayAndroid::Init(int width, int height, int fps)
	{
		DisplayBase::Init(width, height, fps);
	}

	void DisplayAndroid::KeepScreenOn(bool enable)
	{
		java_keep_screen_on(enable);
	}
}