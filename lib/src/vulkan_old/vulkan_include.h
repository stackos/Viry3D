/*
* Viry3D
* Copyright 2014-2019 by Stack - stackos@qq.com
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

#if VR_WINDOWS
#include "vulkan/vulkan.h"
#elif VR_ANDROID
#include "vulkan/vulkan_wrapper/vulkan_wrapper.h"
#elif VR_IOS
#include "vulkan/vulkan.h"
#include "vulkan/vulkan_ios.h"
#elif VR_MAC
#include "vulkan/vulkan.h"
#include "vulkan/vulkan_macos.h"
#endif
