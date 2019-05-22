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

#include "private/backend/Driver.h"
#include "DriverBase.h"
#include <mutex>
#include <unordered_map>

namespace filament
{
	namespace backend
	{
		class D3D11Platform;
		class D3D11Context;

		class D3D11Driver final : public DriverBase
		{
		public:
			static Driver* create(backend::D3D11Platform* platform);

		private:
			explicit D3D11Driver(backend::D3D11Platform* platform) noexcept;
			~D3D11Driver() noexcept override;
			ShaderModel getShaderModel() const noexcept final;

			template<typename T>
			friend class backend::ConcreteDispatcher;

#define DECL_DRIVER_API(methodName, paramsDecl, params) \
    UTILS_ALWAYS_INLINE inline void methodName(paramsDecl);

#define DECL_DRIVER_API_SYNCHRONOUS(RetType, methodName, paramsDecl, params) \
    RetType methodName(paramsDecl) override;

#define DECL_DRIVER_API_RETURN(RetType, methodName, paramsDecl, params) \
    RetType methodName##S() noexcept override; \
    UTILS_ALWAYS_INLINE inline void methodName##R(RetType, paramsDecl);

#include "private/backend/DriverAPI.inc"

		using Blob = void*;
		using HandleMap = std::unordered_map<HandleBase::HandleId, Blob>;

		template<typename Dp, typename B>
		Handle<B> alloc_handle()
		{
			std::lock_guard<std::mutex> lock(m_handle_map_mutex);
			m_handle_map[m_next_id] = malloc(sizeof(Dp));
			return Handle<B>(m_next_id++);
		}

		template<typename Dp, typename B, typename ... ARGS>
		Dp* construct_handle(HandleMap& handle_map, Handle<B>& handle, ARGS&& ... args) noexcept
		{
			std::lock_guard<std::mutex> lock(m_handle_map_mutex);
			auto iter = handle_map.find(handle.getId());
			assert(iter != handle_map.end());
			Blob& blob = iter->second;
			Dp* addr = reinterpret_cast<Dp*>(blob);
			new(addr) Dp(std::forward<ARGS>(args)...);
			return addr;
		}

		template<typename Dp, typename B>
		void destruct_handle(HandleMap& handle_map, Handle<B>& handle) noexcept
		{
			if (handle)
			{
				std::lock_guard<std::mutex> lock(m_handle_map_mutex);
				assert(handle);
				// Call the destructor, remove the blob, don't bother reclaiming the integer id.
				auto iter = handle_map.find(handle.getId());
				assert(iter != handle_map.end());
				Blob& blob = iter->second;
				reinterpret_cast<Dp*>(blob)->~Dp();
				free(blob);
				handle_map.erase(handle.getId());
			}
		}

		template<typename Dp, typename B>
		Dp* handle_cast(HandleMap& handle_map, Handle<B>& handle) noexcept
		{
			if (handle)
			{
				std::lock_guard<std::mutex> lock(m_handle_map_mutex);
				assert(handle);
				auto iter = handle_map.find(handle.getId());
				assert(iter != handle_map.end());
				Blob& blob = iter->second;
				return reinterpret_cast<Dp*>(blob);
			}
			else
			{
				return nullptr;
			}
		}

		template<typename Dp, typename B>
		const Dp* handle_const_cast(HandleMap& handle_map, const Handle<B>& handle) noexcept
		{
			if (handle)
			{
				std::lock_guard<std::mutex> lock(m_handle_map_mutex);
				assert(handle);
				auto iter = handle_map.find(handle.getId());
				assert(iter != handle_map.end());
				Blob& blob = iter->second;
				return reinterpret_cast<const Dp*>(blob);
			}
			else
			{
				return nullptr;
			}
		}

		private:
			backend::D3D11Platform& m_platform;
			std::mutex m_handle_map_mutex;
			HandleMap m_handle_map;
			HandleBase::HandleId m_next_id = 1;
			D3D11Context* m_context;
		};
	}
}
