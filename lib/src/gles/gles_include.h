#pragma once

#if VR_IOS
#include <OpenGLES/ES3/gl.h>
#elif VR_ANDROID
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#elif VR_WINDOWS
#include <Windows.h>
#include "glew/include/GL/glew.h"
#include "glew/include/GL/wglew.h"
#endif