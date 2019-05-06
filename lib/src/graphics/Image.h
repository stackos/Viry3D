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

#include "Object.h"

namespace Viry3D
{
    enum class ImageFormat
    {
        None = 0,
        R8,
        R8G8B8,
        R8G8B8A8,
    };

	class Image : public Object
	{
	public:
        static Ref<Image> LoadFromFile(const String& path);
		static Ref<Image> LoadJPEG(const ByteBuffer& jpeg);
		static Ref<Image> LoadPNG(const ByteBuffer& png);
		void EncodeToPNG(const String& file);

        int width = 0;
        int height = 0;
        ImageFormat format = ImageFormat::None;
        ByteBuffer data;
	};
}
