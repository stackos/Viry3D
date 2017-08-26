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
#include "container/Map.h"
#include "container/Vector.h"
#include "memory/Ref.h"

#define DECLARE_COM_BASE(CBase) \
    public: \
        typedef void* (*ClassGen)(); \
        static CBase* Create(String class_name) \
		{ \
            return (CBase*) GetClassMap()[class_name](); \
        } \
		static String ClassName() { return #CBase; } \
		static const Vector<String>& ClassNames() { \
			if(m_class_names.Empty()) { \
				m_class_names.Add(#CBase); \
			} \
			return m_class_names; \
		} \
        virtual String GetTypeName() const { return #CBase; } \
		virtual const Vector<String>& GetClassNames() const { return CBase::ClassNames(); } \
		virtual void DeepCopy(const Ref<Object>& source); \
	protected: \
		static void Register(String class_name, ClassGen class_gen) \
		{ \
			GetClassMap().Add(class_name, class_gen); \
		} \
    private: \
        static Map<String, ClassGen>& GetClassMap() \
		{ \
            return m_class_map; \
        } \
        static Map<String, ClassGen> m_class_map; \
		static Vector<String> m_class_names;

#define DEFINE_COM_BASE(CBase) \
    Map<String, CBase::ClassGen> CBase::m_class_map; \
	Vector<String> CBase::m_class_names;

#define DECLARE_COM_CLASS(CDerived, CSuper) \
    public: \
		static void RegisterComponent() \
		{ \
			Register(#CDerived, CDerived::Create); \
		} \
		static String ClassName() { return #CDerived; } \
		static const Vector<String>& ClassNames() { \
			if(m_class_names.Empty()) { \
				m_class_names = CSuper::ClassNames(); \
				m_class_names.Add(#CDerived); \
			} \
			return m_class_names; \
		} \
        virtual String GetTypeName() const { return #CDerived; } \
		virtual const Vector<String>& GetClassNames() const { return CDerived::ClassNames(); } \
		virtual void DeepCopy(const Ref<Object>& source); \
    private: \
        static void* Create() \
		{ \
            return new CDerived(); \
        } \
		static Vector<String> m_class_names;

#define DECLARE_COM_CLASS_ABSTRACT(CDerived, CSuper) \
    public: \
		static String ClassName() { return #CDerived; } \
		static const Vector<String>& ClassNames() { \
			if(m_class_names.Empty()) { \
				m_class_names = CSuper::ClassNames(); \
				m_class_names.Add(#CDerived); \
			} \
			return m_class_names; \
		} \
        virtual String GetTypeName() const { return #CDerived; } \
		virtual const Vector<String>& GetClassNames() const { return CDerived::ClassNames(); } \
		virtual void DeepCopy(const Ref<Object>& source); \
    private: \
		static Vector<String> m_class_names;

#define DEFINE_COM_CLASS(CDerived) \
    Vector<String> CDerived::m_class_names;
