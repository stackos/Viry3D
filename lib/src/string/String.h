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

#include "container/Vector.h"
#include "memory/ByteBuffer.h"
#include <string>
#include <sstream>

namespace Viry3D
{
	class String
	{
	public:
		static String Format(const char* format, ...);
		static String Base64(const char* bytes, int size);
		static String Utf8ToGb2312(const String& str);
		static String Gb2312ToUtf8(const String& str);
		static String UrlDecode(const String& str);

		String();
		String(const char* str);
		String(const char* str, int size);
		String(const ByteBuffer& buffer);
		String(const char32_t* unicode32);

		int Size() const;
		bool Empty() const;

		int IndexOf(const String& str, int start = 0) const;
		int LastIndexOf(const String& str, int start = -1) const;
		String Replace(const String& old, const String& to) const;
		Vector<String> Split(const String& separator, bool exclude_empty = false) const;
		bool StartsWith(const String& str) const;
		bool EndsWith(const String& str) const;
		String Substring(int start, int count = -1) const;
		bool Contains(const String& str) const;
		Vector<char32_t> ToUnicode32() const;
		String ToLower() const;
		String ToUpper() const;

		bool operator ==(const String& right) const;
		bool operator !=(const String& right) const;
		String operator +(const String& right) const;
		String& operator +=(const String& right);
		bool operator <(const String& right) const;
		char& operator[](int index);
		const char& operator[](int index) const;

		const char* CString() const;

		template<class V>
		V To() const;

		template<class V>
		static String ToString(const V& v);

	private:
		std::string m_string;
	};

	bool operator ==(const char* left, const String& right);
	bool operator !=(const char* left, const String& right);
	String operator +(const char* left, const String& right);

	template<class V>
	V String::To() const
	{
		std::stringstream ss(m_string);
		V v;
		ss >> v;
		return v;
	}

	template<class V>
	String String::ToString(const V& v)
	{
		std::stringstream ss;
		ss << v;
		String str;
		str.m_string = ss.str();
		return str;
	}
}
