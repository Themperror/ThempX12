#pragma once
#include <string>
namespace Themp
{
	namespace Util
	{
		void SetLogFile(const char* filePath);
	}
	void Print(const char* message, ...);
	void Print(const wchar_t* message, ...);
	void Print(const std::string message, ...);
}