#include "material.h"
#include "util/print.h"
#include "util/break.h"

#include <algorithm>
#include <sstream>
#include <string_view>

using namespace Themp::D3D;

bool Material::ParseMaterialFile(const std::string& data)
{
	enum MaterialMembers
	{
		MAIN_PASS,
		SHADER,
		COUNT
	};

	const std::string_view validEntries[MaterialMembers::COUNT]
	{
		"mainpass",
		"shader",
	};

	std::string config = data;
	for (size_t i = 0; i < config.size(); i++)
	{
		config[i] = std::tolower(config[i]);
	}

	std::stringstream stream(config, std::ios::in);
	
	std::string line(1024,'\0');
	while (std::getline(stream, line))
	{
		auto noWinNewLines = std::remove_if(line.begin(), line.end(), [](char c) { return c == '\r'; });
		line.erase(noWinNewLines, line.end());
		if (line.size() > 1)
		{
			//comment
			if (line[0] == '#')
			{
				continue;
			}

			size_t equalsIndex = line.find('=');
			if (equalsIndex != std::string::npos)
			{
				std::string_view prefix = std::string_view(line.data(), equalsIndex);
				std::string_view suffix = std::string_view(line.data() + equalsIndex+1, line.size() - (equalsIndex));
				
				if (prefix == validEntries[MaterialMembers::MAIN_PASS])
				{
					
				}
				else if (prefix == validEntries[MaterialMembers::SHADER])
				{

				}
			}
			else
			{
				Themp::Print("Malformed configuration line: [%s]", line.c_str());
				Themp::Break();
			}
		}
	}
	return true;
}