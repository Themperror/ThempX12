#include "stringUtils.h"

std::string Themp::Util::ToLowerCase(const std::string& string)
{
	std::string newString = string;
	for (size_t i = 0; i < newString.size(); i++)
	{
		newString[i] = std::tolower(newString[i]);
	}
	return newString;
}

std::wstring Themp::Util::ToWideString(const std::string& string)
{
	std::wstring newString(string.size()*2, '\0');
	size_t numCharsConverted = 0;
	mbstowcs_s(&numCharsConverted, newString.data(), newString.size(), string.data(), string.size());
	newString.resize(numCharsConverted-1);
	return newString;
}
