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