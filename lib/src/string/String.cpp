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

#include "String.h"
#include "memory/Memory.h"
#include <stdarg.h>

#if VR_WINDOWS
#include <Windows.h>
#endif

namespace Viry3D
{
	static const char BASE64_TABLE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	String String::Format(const char* format, ...)
	{
		String result;

		va_list vs;
		va_start(vs, format);
		int size = vsnprintf(nullptr, 0, format, vs);
		va_end(vs);

		char* buffer = Memory::Alloc<char>(size + 1);
		buffer[size] = 0;

		va_start(vs, format);
		size = vsnprintf(buffer, size + 1, format, vs);
		va_end(vs);

		result.m_string = buffer;

		Memory::Free(buffer);

		return result;
	}

	String String::Base64(const char* bytes, int size)
	{
		int size_pad = size;
		if (size_pad % 3 != 0)
		{
			size_pad += 3 - (size_pad % 3);
		}
		int round = size_pad / 3;

		std::string str(round * 4, '\0');

		int index;
		char a, b, c;
		for (int i = 0; i < round; ++i)
		{
			a = 0; b = 0; c = 0;

			index = i * 3 + 0;
			if (index < size) a = bytes[index];
			index = i * 3 + 1;
			if (index < size) b = bytes[index];
			index = i * 3 + 2;
			if (index < size) c = bytes[index];

			str[i * 4 + 0] = BASE64_TABLE[(a & 0xfc) >> 2];
			str[i * 4 + 1] = BASE64_TABLE[((a & 0x3) << 4) | ((b & 0xf0) >> 4)];
			str[i * 4 + 2] = BASE64_TABLE[((b & 0xf) << 2) | ((c & 0xc0) >> 6)];
			str[i * 4 + 3] = BASE64_TABLE[c & 0x3f];
		}

		for (int i = size_pad - size, j = 0; i > 0; --i, ++j)
		{
			str[(round - 1) * 4 + 3 - j] = '=';
		}

		return String(str.c_str());
	}

	String String::Utf8ToGb2312(const String& str)
	{
#if VR_WINDOWS
		int size = MultiByteToWideChar(CP_UTF8, 0, str.CString(), str.Size(), nullptr, 0);
		wchar_t* wstr = (wchar_t*) calloc(1, (size + 1) * 2);
		MultiByteToWideChar(CP_UTF8, 0, str.CString(), str.Size(), wstr, size);

		size = WideCharToMultiByte(CP_ACP, 0, wstr, size, nullptr, 0, nullptr, false);
		char* cstr = (char*) calloc(1, size + 1);
		WideCharToMultiByte(CP_ACP, 0, wstr, size, cstr, size, nullptr, false);

		String ret = cstr;

		free(cstr);
		free(wstr);

		return ret;
#else
		return str;
#endif
	}

	String String::Gb2312ToUtf8(const String& str)
	{
#if VR_WINDOWS
		int size = MultiByteToWideChar(CP_ACP, 0, str.CString(), str.Size(), nullptr, 0);
		wchar_t* wstr = (wchar_t*) calloc(1, (size + 1) * 2);
		MultiByteToWideChar(CP_ACP, 0, str.CString(), str.Size(), wstr, size);

		size = WideCharToMultiByte(CP_UTF8, 0, wstr, size, nullptr, 0, nullptr, false);
		char* cstr = (char*) calloc(1, size + 1);
		WideCharToMultiByte(CP_UTF8, 0, wstr, size, cstr, size, nullptr, false);

		String ret = cstr;

		free(cstr);
		free(wstr);

		return ret;
#else
		return str;
#endif
	}

	String String::UrlDecode(const String& str)
	{
		std::string dest = str.CString();

		int i = 0;
		int j = 0;
		char c;
		int size = (int) str.Size();
		while (i < size)
		{
			c = str[i];
			switch (c)
			{
				case '+':
					dest[j++] = ' ';
					i++;
					break;
				case '%':
				{
					while (i + 2 < size && c == '%')
					{
						auto sub = str.Substring(i + 1, 2);
						char v = (char) strtol(sub.CString(), nullptr, 16);
						dest[j++] = v;
						i += 3;
						if (i < size)
						{
							c = str[i];
						}
					}
				}
				break;
				default:
					dest[j++] = c;
					i++;
					break;
			}
		}

		return String(dest.c_str(), j);
	}

	String::String()
	{
	}

	String::String(const char *str):
		m_string(str)
	{
	}

	String::String(const char* str, int size) :
		m_string(str, size)
	{
	}

	String::String(const ByteBuffer& buffer) :
		m_string((const char*) buffer.Bytes(), buffer.Size())
	{
	}

	int String::Size() const
	{
		return (int) m_string.size();
	}

	bool String::Empty() const
	{
		return m_string.empty();
	}

	bool String::operator ==(const String& right) const
	{
		if (this->Size() == right.Size())
		{
			return Memory::Compare(this->m_string.data(), right.m_string.data(), (int) m_string.size()) == 0;
		}

		return false;
	}

	bool String::operator !=(const String& right) const
	{
		return !(*this == right);
	}

	bool operator ==(const char* left, const String& right)
	{
		return right == left;
	}

	bool operator !=(const char* left, const String& right)
	{
		return right != left;
	}

	String String::operator +(const String& right) const
	{
		String result;
		result.m_string = m_string + right.m_string;
		return result;
	}

	String& String::operator +=(const String& right)
	{
		*this = *this + right;
		return *this;
	}

	String operator +(const char* left, const String& right)
	{
		return String(left) + right;
	}

	bool String::operator <(const String& right) const
	{
		return m_string < right.m_string;
	}

	char& String::operator[](int index)
	{
		return m_string[index];
	}

	const char& String::operator[](int index) const
	{
		return m_string[index];
	}

	const char* String::CString() const
	{
		return m_string.c_str();
	}

	int String::IndexOf(const String& str, int start) const
	{
        size_t pos = m_string.find(str.m_string, start);
        if (pos != std::string::npos)
        {
            return (int) pos;
        }
        else
        {
            return -1;
        }
	}

	bool String::Contains(const String& str) const
	{
		return this->IndexOf(str) >= 0;
	}

	int String::LastIndexOf(const String& str, int start) const
	{
        size_t pos = m_string.rfind(str.m_string, start);
        if (pos != std::string::npos)
        {
            return (int) pos;
        }
        else
        {
            return -1;
        }
	}

	String String::Replace(const String& old, const String& to) const
	{
		String result(*this);

		int start = 0;
		while (true)
		{
			int index = result.IndexOf(old, start);
			if (index >= 0)
			{
				result.m_string.replace(index, old.m_string.size(), to.m_string);
				start = index + (int) to.m_string.size();
			}
			else
			{
				break;
			}
		}

		return result;
	}

	Vector<String> String::Split(const String& separator, bool exclude_empty) const
	{
		Vector<String> result;

		int start = 0;
		while (true)
		{
			int index = this->IndexOf(separator, start);
			if (index >= 0)
			{
				String str = this->Substring(start, index - start);
				if (!str.Empty() || !exclude_empty)
				{
					result.Add(str);
				}
				start = index + separator.Size();
			}
			else
			{
				break;
			}
		}

		String str = this->Substring(start, -1);
		if (!str.Empty() || !exclude_empty)
		{
			result.Add(str);
		}

		return result;
	}

	bool String::StartsWith(const String& str) const
	{
        if (str.Size() == 0)
        {
            return true;
        }
        else if (this->Size() < str.Size())
        {
            return false;
        }
        else
        {
            return Memory::Compare(&(*this)[0], &str[0], str.Size()) == 0;
        }
	}

	bool String::EndsWith(const String& str) const
	{
        if (str.Size() == 0)
        {
            return true;
        }
        else if (this->Size() < str.Size())
        {
            return false;
        }
        else
        {
            return Memory::Compare(&(*this)[this->Size() - str.Size()], &str[0], str.Size()) == 0;
        }
	}

	String String::Substring(int start, int count) const
	{
		String result;
		result.m_string = m_string.substr(start, count);
		return result;
	}

	static int Utf8ToUnicode32(const char* utf8, char32_t& c32)
	{
		int byte_count = 0;

		for (int i = 0; i < 8; ++i)
		{
			unsigned char c = utf8[0];

			if (((c << i) & 0x80) == 0)
			{
				if (i == 0)
				{
					byte_count = 1;
				}
				else
				{
					byte_count = i;
				}
				break;
			}
		}

		if (byte_count >= 1 && byte_count <= 6)
		{
			char32_t code = 0;

			for (int i = 0; i < byte_count; ++i)
			{
				unsigned int c = utf8[i];
				unsigned char part;

				if (i == 0)
				{
					part = (c << (byte_count + 24)) >> (byte_count + 24);
				}
				else
				{
					part = c & 0x3f;
				}

				code = (code << 6) | part;
			}

			c32 = code;

			return byte_count;
		}
		else
		{
			return 0;
		}
	}

	static Vector<char> Unicode32ToUtf8(char32_t c32)
	{
		Vector<char> buffer;
		int byte_count = 0;

		if (c32 <= 0x7f)
		{
			byte_count = 1;
		}
		else if (c32 <= 0x7ff)
		{
			byte_count = 2;
		}
		else if (c32 <= 0xffff)
		{
			byte_count = 3;
		}
		else if (c32 <= 0x1fffff)
		{
			byte_count = 4;
		}
		else if (c32 <= 0x3ffffff)
		{
			byte_count = 5;
		}
		else if (c32 <= 0x7fffffff)
		{
			byte_count = 6;
		}

		std::vector<char> bytes;
		for (int i = 0; i < byte_count - 1; ++i)
		{
			bytes.push_back((c32 & 0x3f) | 0x80);
			c32 >>= 6;
		}

		if (byte_count > 1)
		{
			bytes.push_back((char) (c32 | (0xffffff80 >> (byte_count - 1))));
		}
		else
		{
			bytes.push_back((char) (c32));
		}

		for (int i = 0; i < byte_count; ++i)
		{
			buffer.Add(bytes[byte_count - 1 - i]);
		}

		return buffer;
	}

	Vector<char32_t> String::ToUnicode32() const
	{
		Vector<char32_t> unicode;

		int size = (int) m_string.size();

		for (int i = 0; i < size; ++i)
		{
			char32_t unicode32 = 0;
			int byte_count = Utf8ToUnicode32(&m_string[i], unicode32);

			if (byte_count > 0)
			{
				unicode.Add(unicode32);

				i += byte_count - 1;
			}
			else
			{
				break;
			}
		}

		return unicode;
	}

	String::String(const char32_t* unicode32)
	{
		Vector<char> str;

		for (int i = 0; unicode32[i] != 0; ++i)
		{
			char32_t c32 = unicode32[i];

			auto bytes = Unicode32ToUtf8(c32);
			str.AddRange(&bytes[0], bytes.Size());
		}

		str.Add(0);

		m_string = &str[0];
	}

	String String::ToLower() const
	{
		Vector<char> str;

		for (auto c : m_string)
		{
			if (c >= 'A' && c <= 'Z')
			{
				c -= 'A' - 'a';
			}
			str.Add(c);
		}
		str.Add(0);

		return String(&str[0]);
	}

	String String::ToUpper() const
	{
		Vector<char> str;

		for (auto c : m_string)
		{
			if (c >= 'a' && c <= 'z')
			{
				c += 'A' - 'a';
			}
			str.Add(c);
		}
		str.Add(0);

		return String(&str[0]);
	}
}
