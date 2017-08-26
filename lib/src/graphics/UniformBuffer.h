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

#pragma once

#if VR_VULKAN
#include "vulkan/BufferVulkan.h"
#elif VR_GLES
#include "gles/BufferGLES.h"
#endif

#include "memory/Ref.h"

namespace Viry3D
{
#if VR_VULKAN
	class UniformBuffer: public BufferVulkan
	{
#elif VR_GLES
	class UniformBuffer: public BufferGLES
	{
#endif
	public:
		static Ref<UniformBuffer> Create(int size);

	private:
		UniformBuffer() { }
	};
}
