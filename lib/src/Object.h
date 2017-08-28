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

#include "string/String.h"
#include "memory/Ref.h"
#include "container/Map.h"
#include "thread/Thread.h"
#include "Debug.h"
#include "Profiler.h"
#include <assert.h>

namespace Viry3D
{
	//
	//	属性:
	//	使用要求:
	//		继承 Object 的类可用 PROPERTY_BEGIN 和 PROPERTY_END 配对定义属性，
	//		如有定义，则应在构造函数内调用 InitProperties
	//	用法:
	//		在 PROPERTY_BEGIN 和 PROPERTY_END 之间用 PROPERTY 定义属性
	//		在后面定义 PROPERTY_GET 或 PROPERTY_GET_SET 来实现属性的访问函数
	//
	class Object
	{
	public:
		static void Init();
		static void Deinit();
		//
		//	基于路径的资源对象缓存, 线程安全
		//	
		static Ref<Object> GetCache(String path);
		//
		//	线程安全
		//
		static void AddCache(String path, const Ref<Object>& obj);

		Object();
		virtual ~Object();
		virtual void DeepCopy(const Ref<Object>& source);
		int GetId() const { return m_id; }
		const String& GetName() const { return m_name; }
		void SetName(const String& name) { m_name = name; }

	protected:
		int m_id;
		String m_name;

	private:
		Object(const Object& obj) { }
		Object& operator =(const Object& obj) { return *this; }

		static Map<String, Ref<Object>> m_cache;
		static Mutex m_mutex;
	};
}
