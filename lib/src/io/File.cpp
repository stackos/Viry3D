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

#include "File.h"
#include "Directory.h"
#include "Debug.h"
#include "zlib/unzip.h"
#include <fstream>

#if VR_WINDOWS
#include <Windows.h>
#endif

namespace Viry3D
{
#if VR_UWP
    extern bool FileExist(const String& path);
    extern ByteBuffer FileReadAllBytes(const String& path);
    extern bool FileWriteAllBytes(const String& path, const ByteBuffer& buffer);

    bool File::Exist(const String& path)
    {
        return FileExist(path);
    }

    ByteBuffer File::ReadAllBytes(const String& path)
    {
        return FileReadAllBytes(path);
    }

    bool File::WriteAllBytes(const String& path, const ByteBuffer& buffer)
    {
        return FileWriteAllBytes(path, buffer);
    }
#else
    bool File::Exist(const String& path)
    {
        std::ifstream is(path.CString(), std::ios::binary);

        bool exist = !(!is);

        if (exist)
        {
            is.close();
        }

        return exist;
    }

    ByteBuffer File::ReadAllBytes(const String& path)
    {
        ByteBuffer buffer;

        std::ifstream is(path.CString(), std::ios::binary);
        if (is)
        {
            is.seekg(0, std::ios::end);
            int size = (int) is.tellg();
            is.seekg(0, std::ios::beg);

            buffer = ByteBuffer(size);

            is.read((char*) buffer.Bytes(), size);
            is.close();
        }

        return buffer;
    }

    bool File::WriteAllBytes(const String& path, const ByteBuffer& buffer)
    {
        std::ofstream os(path.CString(), std::ios::binary);
        if (os)
        {
            os.write((const char*) buffer.Bytes(), buffer.Size());
            os.close();

            return true;
        }
        else
        {
            return false;
        }
    }
#endif

	String File::ReadAllText(const String& path)
	{
		return String(File::ReadAllBytes(path));
	}

    bool File::WriteAllText(const String& path, const String& text)
	{
		ByteBuffer buffer((byte*) text.CString(), text.Size());
        return File::WriteAllBytes(path, buffer);
	}

    void File::Delete(const String& path)
    {
#if VR_WINDOWS
        ::DeleteFile(path.CString());
#endif
    }

	static void UnzipFile(unzFile file, const String& path)
	{
		auto dir = path.Substring(0, path.LastIndexOf("/"));
		Directory::Create(dir);

		int result = unzOpenCurrentFilePassword(file, nullptr);

		ByteBuffer buffer(8192);

		std::ofstream os(path.CString(), std::ios::out | std::ios::binary);

		if (os)
		{
			do
			{
				result = unzReadCurrentFile(file, buffer.Bytes(), buffer.Size());
				if (result > 0)
				{
					os.write((const char*) buffer.Bytes(), result);
				}
			} while (result > 0);

			os.close();
		}

		unzCloseCurrentFile(file);
	}

	void File::Unzip(const String& path, const String& source, const String& dest, bool directory)
	{
		auto file = unzOpen64(path.CString());
		if (file)
		{
			unz_file_info64 file_info;
			char filename_inzip[256];

			auto result = unzGoToFirstFile(file);
			while (result == UNZ_OK)
			{
				result = unzGetCurrentFileInfo64(file, &file_info, filename_inzip, sizeof(filename_inzip), nullptr, 0, nullptr, 0);
				if (result != UNZ_OK)
				{
					break;
				}

				if (directory)
				{
					String filename(filename_inzip);
					if (filename.StartsWith(source))
					{
						String dest_filename = dest + filename.Substring(source.Size());
                        UnzipFile(file, dest_filename);
					}
				}
				else
				{
					if (source == filename_inzip)
					{
                        UnzipFile(file, dest);
						break;
					}
				}

				result = unzGoToNextFile(file);
			}

			unzClose(file);
		}
		else
		{
			Log("zip file open failed:%s", path.CString());
		}
	}
}
