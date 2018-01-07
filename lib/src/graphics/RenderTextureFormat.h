/*
* Viry3D
* Copyright 2014-2018 by Stack - stackos@qq.com
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
	enum class RenderTextureFormat
	{
		RGBA32 = 0,			//Color render texture format, 8 bits per channel
		Depth = 1,			//A depth render texture format
		RGBAHalf = 2,		//Color render texture format, 16 bit floating point per channel
//		RGB565 = 4,
//		ARGB4444 = 5,
//		ARGB1555 = 6,
//		Default = 7,
//		DefaultHDR = 9,
//		RGBAFloat = 11,		//Color render texture format, 32 bit floating point per channel
//      RGFloat = 12,
		RGHalf = 13,
		RFloat = 14,		//Scalar (R) render texture format, 32 bit floating point
//		RHalf = 15,
		R8 = 16,			//Scalar (R) render texture format, 8 bit fixed point
//		ARGBInt = 17,		//Four channel (ARGB) render texture format, 32 bit signed integer per channel
//		RGInt = 18,
//		RInt = 19,
	};
}
