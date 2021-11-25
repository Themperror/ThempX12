#pragma once

#include <string>
#include <util/print.h>
#include <util/break.h>
#include <charconv>
namespace Themp::Util
{
	std::string ToLowerCase(const std::string& string);

	template<class T, class S>
	T FromString(const S& s)
	{
		T result{};
		auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), result);
		if (ec == std::errc())
		{
			return result;
		}
		Themp::Print("Failed to convert %*s", s.size(), s.data());
		Themp::Break();
		return result;
	}

}