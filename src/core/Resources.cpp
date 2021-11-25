#include "Engine.h"
#include "Resources.h"
#include "renderer/control.h"
#include "util/print.h"
#include "util/break.h"
#include "util/stringUtils.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <charconv>
#include <optional>
#include <sstream>

namespace Themp
{
	std::unique_ptr<Resources> Resources::instance;

	#define RESOURCES_FOLDER L"../resources/"
	#define MATERIALS_FOLDER RESOURCES_FOLDER"materials/"
	#define PASSES_FOLDER RESOURCES_FOLDER"passes/"
	#define SHADERS_FOLDER RESOURCES_FOLDER"shaders/"

	std::vector<std::wstring> LoadFilesFromDirectory(std::wstring dir)
	{
		//warning, WIN32 API ahead :)
		std::vector<std::wstring> files;
		WIN32_FIND_DATAW ffd;
		HANDLE hFind = FindFirstFileW((dir + L"*").c_str(), &ffd);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			Themp::Print("Something went wrong %S", dir.c_str());
		}
		while (FindNextFileW(hFind, &ffd) != 0)
		{
			// ignore directories
			if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				// create a full path for each file we find, e.g. "c:\indir\foo.txt"
				std::wstring file_path;
				file_path.reserve(2048);
				file_path = dir + ffd.cFileName;
				std::transform(file_path.begin(), file_path.end(), file_path.begin(), ::towlower);
				files.push_back(file_path);
			}
		}
		FindClose(hFind);
		return files;
	}


	std::optional<int> TryParseSectionNumber(std::string_view name, std::string_view source)
	{
		if (source.size() <= name.size())
		{
			return std::nullopt;
		}

		auto [name_it, _] = std::mismatch(name.begin(), name.end(), source.begin());
		if (name_it != name.end())
		{
			return std::nullopt;
		}

		const auto number_sv = source.substr(name.size());

		int number_result{};
		auto [ptr, ec] = std::from_chars(number_sv.data(), number_sv.data() + number_sv.size(), number_result);
		if (ec != std::errc())
		{
			return std::nullopt;
		}

		return number_result;
	}

	std::string ReadFileToString(const std::wstring& filePath)
	{
		HANDLE file = CreateFileW(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (file == INVALID_HANDLE_VALUE)
		{
			Themp::Print("Was unable to open the file: [%S]", filePath.c_str());
			Themp::Break();
			return "";
		}

		DWORD fileSize = GetFileSize(file, NULL);
		std::string data(fileSize,'\0');
		DWORD readBytes = 0;
		if (!ReadFile(file, data.data(), fileSize, &readBytes, NULL))
		{
			Themp::Print("Was unable to read the file: [%S]", filePath.c_str());
			Themp::Break();
		}
		if (readBytes != fileSize)
		{
			Themp::Print("Was unable to read the entire file: [%S]", filePath.c_str());
			Themp::Break();
		}

		return data;
	}

	void Resources::LoadMaterials()
	{
		DWORD attributes = GetFileAttributesW(MATERIALS_FOLDER);
		if (attributes == INVALID_FILE_ATTRIBUTES)
		{
			Themp::Print(L"Unable to find material folder at " MATERIALS_FOLDER);
			Themp::Break();
			return;
		}
		std::vector<std::wstring> materials = LoadFilesFromDirectory(MATERIALS_FOLDER);

		std::string materialData(10240, '0');
		for (const std::wstring& path : materials)
		{
			Themp::Print("Loading material file: [%S]", path.c_str());
			auto passes = LoadMaterial( ReadFileToString(path) );
			MergePasses(passes);
		}
	}


	void Resources::AddOrSetShaderToPass(std::string_view name, int index, Themp::D3D::Pass& pass)
	{

	}

	D3D::Pass Resources::LoadPass(std::string_view filename) const
	{
		using namespace Themp::D3D;

		Pass pass{ filename };

		//std::format is C++20 :(
		const wchar_t* extension = L".pass";
		std::wstring path = std::wstring(PASSES_FOLDER);
		path.reserve(filename.size() + wcslen(extension) + path.size());
		path.append(filename.begin(), filename.end())
			.append(extension);

		std::string data = ReadFileToString(path);

		std::stringstream stream(Themp::Util::ToLowerCase(data), std::ios::in);
		std::string line(1024, '\0');
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
					std::string_view suffix = std::string_view(line.data() + equalsIndex + 1, line.size() - (equalsIndex) - 1);

					if (prefix == Pass::GetPassConfigurationMembersAsString(Pass::PassConfigurationMembers::PRIORITY))
					{
						int value = Themp::Util::FromString<int, std::string_view>(suffix);
					}
					else if (prefix == Pass::GetPassConfigurationMembersAsString(Pass::PassConfigurationMembers::DEPTHTARGET))
					{
						//LoadRenderTarget(RenderTarget::DSV, format, resolution_mode, scaler)
					}
					else if (auto colorTargetIndex = TryParseSectionNumber(Pass::GetPassConfigurationMembersAsString(Pass::PassConfigurationMembers::COLORTARGET), prefix))
					{
						//LoadRenderTarget(RenderTarget::RTV, format, resolution_mode, scaler)
					}
				}
				else
				{
					Themp::Print("Malformed configuration line: [%s]", line.c_str());
					Themp::Break();
				}
			}
		}
		return pass;
	}

	D3D::ShaderHandle Resources::LoadShader(std::string_view filename)
	{
		return 0;
	}
	
	void Resources::MergePasses(std::vector<Themp::D3D::Pass> passes)
	{
		
	}

	std::vector<Themp::D3D::Pass> Resources::LoadMaterial(const std::string& data)
	{
		using namespace Themp::D3D;
		enum MaterialMembers
		{
			PASS,
			SHADER,
			COUNT
		};

		const std::string_view validEntries[MaterialMembers::COUNT]
		{
			"pass",
			"shader",
		};

		std::vector<std::pair<Pass, int>> passData;

		std::stringstream stream(Themp::Util::ToLowerCase(data), std::ios::in);

		std::string line(1024, '\0');
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
					std::string_view suffix = std::string_view(line.data() + equalsIndex + 1, line.size() - (equalsIndex) - 1);

					if (auto mainpass = TryParseSectionNumber(validEntries[MaterialMembers::PASS], prefix))
					{
						bool foundPass = false;
						Pass pass = LoadPass(suffix);
						for (int i = 0; i < passData.size(); i++)
						{
							if (passData[i].second == mainpass)
							{
								foundPass = true;
								passData[i].first = pass;
								break;
							}
						}
						if (!foundPass)
						{
							passData.push_back({ pass, mainpass.value() });
						}
					}
					else if (auto shader = TryParseSectionNumber(validEntries[MaterialMembers::SHADER], prefix))
					{
						bool foundPass = false;
						ShaderHandle shaderHandle = LoadShader(suffix);
						for (int i = 0; i < passData.size(); i++)
						{
							if (passData[i].second == shader)
							{
								foundPass = true;
								passData[i].first.AddRenderable(shaderHandle);
								break;
							}
						}
						if (!foundPass)
						{
							Pass pass{ "" };
							pass.AddRenderable(shaderHandle);
							passData.push_back({ pass, shader.value() });
						}
					}
				}
				else
				{
					Themp::Print("Malformed configuration line: [%s]", line.c_str());
					Themp::Break();
				}
			}
		}

		std::sort(passData.begin(), passData.end(), [](const std::pair<Pass, int>& a, const std::pair<Pass, int>& b) { return a.second < b.second; });

		std::vector<Pass> passes;
		for (auto& pass : passData)
		{
			if (!pass.first.IsValid())
			{
				Themp::Print("Pass %i was not valid!", pass.second);
				Themp::Break();
			}
			passes.push_back(pass.first);
		}
		return passes;
	}

}