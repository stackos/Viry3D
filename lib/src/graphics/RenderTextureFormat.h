#pragma once

namespace Viry3D
{
	struct RenderTextureFormat
	{
		enum Enum
		{
			RGBA32 = 0,			//Color render texture format, 8 bits per channel
			Depth = 1,			//A depth render texture format
			RGBAHalf = 2,		//Color render texture format, 16 bit floating point per channel
								//			RGB565 = 4,
								//			ARGB4444 = 5,
								//			ARGB1555 = 6,
								//			Default = 7,
								//			DefaultHDR = 9,
								//          RGBAFloat = 11,		//Color render texture format, 32 bit floating point per channel
								//          RGFloat = 12,
								RGHalf = 13,
								RFloat = 14,		//Scalar (R) render texture format, 32 bit floating point
													//			RHalf = 15,
													//			R8 = 16,			//Scalar (R) render texture format, 8 bit fixed point
													//			ARGBInt = 17,		//Four channel (ARGB) render texture format, 32 bit signed integer per channel
													//			RGInt = 18,
													//			RInt = 19,
		};
	};
}