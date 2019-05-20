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

#include <stdint.h>
#include <backend/DriverEnums.h>
#include <backend/Platform.h>

namespace filament
{
	namespace backend
	{
		class D3D11Platform : public DefaultPlatform
		{
		public:
			~D3D11Platform() override;
		};
	}

	class PlatformD3D11 final : public backend::D3D11Platform
	{
	public:
		backend::Driver* createDriver(void* sharedContext) noexcept override;
		int getOSVersion() const noexcept override { return 0; }
		~PlatformD3D11() override;
	};
}
