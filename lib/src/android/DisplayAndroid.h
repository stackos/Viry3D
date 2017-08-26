#pragma once

#include "graphics/DisplayBase.h"

namespace Viry3D
{
	class DisplayAndroid : public DisplayBase
	{
	public:
		DisplayAndroid() { }
		void Init(int width, int height, int fps);
		void Deinit() { }
		void ProcessSystemEvents() { }
		void KeepScreenOn(bool enable);
	};
}