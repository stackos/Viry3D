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

namespace Viry3D
{
	enum class TextureFormat
	{
		Alpha8,
		ARGB4444,
		RGB24,
		RGBA32,
		ARGB32,
		RGB565,
		DXT1,
		DXT5,
		RGBA4444,
		PVRTC_RGB2,
		PVRTC_RGBA2,
		PVRTC_RGB4,
		PVRTC_RGBA4,
		ETC_RGB4,
		ATC_RGB4,
		ATC_RGBA8,
		BGRA32,
		ATF_RGB_DXT1,
		ATF_RGBA_JPG,
		ATF_RGB_JPG,

		ETC_RGB4_X2,
		PVRTC_RGB4_X2,
	};
}
