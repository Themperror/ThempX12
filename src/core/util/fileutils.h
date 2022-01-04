#pragma once

#include <string>
#include <vector>

namespace Themp::Util
{
	std::string GetPathName(std::string s);
	std::string ReadFileToString(const std::string& filePath);
	std::vector<uint8_t> ReadFileToVector(const std::string& filePath);
	bool WriteFile(const std::vector<uint8_t>& data, const std::string& filePath);
	bool WriteFile(const void* data, size_t size, const std::string& filePath);
	void EnsureFileTreeExists(const std::string& filePath);
	void EnsureFileTreeExists(const std::wstring& filePath);
	std::string GetFileName(std::string_view path);
	std::string ReplaceExtensionWith(std::string str, const std::string& extension);
	std::string RemoveFileFromPathName(std::string str);
	std::vector<std::string> LoadFilesFromDirectory(std::string dir);
	bool FileExists(const std::string& filePath);
}