#include "core/util/fileutils.h"
#include "core/engine.h"
#include "core/util/print.h"
#include "core/util/break.h"
#include <algorithm>
namespace Themp::Util
{
	std::string GetPathName(std::string s)
	{
		std::string name = "";
		int64_t size = s.size() - 1;
		for (int64_t i = size; i >= 0; i--)
		{
			if (s.at(i) == '\\' || s[i] == '/')
			{
				i++;
				name = s.substr(0, i);
				break;
			}
		}
		return name;
	}
	std::vector<std::string> LoadFilesFromDirectory(std::string dir)
	{
		std::vector<std::string> files;

		DWORD attributes = GetFileAttributesA(dir.c_str());
		if (attributes == INVALID_FILE_ATTRIBUTES)
		{
			Themp::Print(L"Unable to find folder at %s", dir.c_str());
			Themp::Break();
			return {};
		}

		WIN32_FIND_DATA ffd{};
		HANDLE hFind = FindFirstFileA((dir + "*").c_str(), &ffd);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			Themp::Print("Something went wrong %s", dir.c_str());
		}
		while (FindNextFileA(hFind, &ffd) != 0)
		{
			// ignore directories
			if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				// create a full path for each file we find, e.g. "c:\indir\foo.txt"
				std::string file_path;
				file_path.reserve(2048);
				file_path = dir + ffd.cFileName;
				std::transform(file_path.begin(), file_path.end(), file_path.begin(), ::tolower);
				files.push_back(file_path);
			}
		}
		FindClose(hFind);
		return files;
	}

	std::string ReadFileToString(const std::string& filePath)
	{
		HANDLE file = CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (file == INVALID_HANDLE_VALUE)
		{
			Themp::Print("Was unable to open the file: [%s]", filePath.c_str());
			Themp::Break();
			return "";
		}

		DWORD fileSize = GetFileSize(file, NULL);
		std::string data(fileSize, '\0');
		DWORD readBytes = 0;
		if (!ReadFile(file, data.data(), fileSize, &readBytes, NULL))
		{
			Themp::Print("Was unable to read the file: [%s]", filePath.c_str());
			Themp::Break();
		}
		if (readBytes != fileSize)
		{
			Themp::Print("Was unable to read the entire file: [%s]", filePath.c_str());
			Themp::Break();
		}
		CloseHandle(file);

		return data;
	}



	std::string GetFileName(std::string_view path)
	{
		std::string filename = std::string(path.begin(), path.end());
		for (size_t i = 0; i < filename.size(); i++)
		{
			if (filename[i] == '/')
			{
				filename[i] = '\\';
			}
		}
		size_t lastBackSlash = filename.find_last_of('\\');
		if (lastBackSlash != std::string::npos && lastBackSlash + 1 < filename.size())
		{
			filename = filename.substr(lastBackSlash + 1, filename.size() - lastBackSlash - 1);
		}

		size_t lastPeriod = filename.find_last_of('.');
		if (lastPeriod != std::string::npos)
		{
			filename = filename.substr(0, lastPeriod);
		}

		return filename;
	}


	std::string ReplaceExtensionWith(std::string str, const std::string& extension)
	{
		int offsetLeadingPeriods = 0;
		for (int i = 0; i < str.size(); i++)
		{
			if (str[i] != '.')
			{
				offsetLeadingPeriods = i;
				break;
			}
		}
		size_t period = str.find_last_of('.');
		if (period != std::string::npos && period > offsetLeadingPeriods)
		{
			str = str.substr(0, period);
		}
		str.append(extension);
		return str;
	}

	std::string RemoveFileFromPath(std::string str)
	{
		for (size_t i = 0; i < str.size(); i++)
		{
			if (str[i] == '/')
			{
				str[i] = '\\';
			}
		}
		size_t lastBackSlash = str.find_last_of('\\');
		if (lastBackSlash != std::string::npos)
		{
			str = str.substr(0, lastBackSlash + 1);
		}
		return str;
	}
}