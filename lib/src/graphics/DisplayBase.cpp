#include "DisplayBase.h"

namespace Viry3D
{
	void DisplayBase::Init(int width, int height, int fps)
	{
		m_width = width;
		m_height = height;
		m_fps = fps;
	}

	void DisplayBase::ProcessSystemEvents()
	{
	}
}