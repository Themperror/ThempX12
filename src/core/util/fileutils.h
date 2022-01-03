#pragma once

#include <string>
#include <vector>

namespace Themp::Util
{
	std::string GetPathName(std::string s);
	std::string ReadFileToString(const std::string& filePath);
	std::string GetFileName(std::string_view path);
	std::string ReplaceExtensionWith(std::string str, const std::string& extension);
	std::string RemoveFileFromPath(std::string str);
	std::vector<std::string> LoadFilesFromDirectory(std::string dir);
}