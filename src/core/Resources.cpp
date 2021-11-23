#include "Engine.h"
#include "Resources.h"
#include "renderer/control.h"
#include "renderer/material.h"
#include "util/print.h"
#include "util/break.h"

#include <iostream>
#include <fstream>
#include <algorithm>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace Themp
{
	std::unique_ptr<Resources> Resources::instance;

	#define RESOURCES_FOLDER L"../resources/"
	#define MATERIALS_FOLDER RESOURCES_FOLDER"materials/"

	std::vector<std::wstring> LoadFilesFromDirectory(std::wstring dir)
	{
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
				std::transform(file_path.begin(), file_path.end(), file_path.begin(), ::towupper);
				files.push_back(file_path);
			}
		}
		FindClose(hFind);
		return files;
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
			HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

			DWORD fileSize = GetFileSize(file, NULL);
			materialData.resize(fileSize);
			DWORD readBytes = 0;
			ReadFile(file, materialData.data(), fileSize, &readBytes, NULL);
			if (readBytes != fileSize)
			{
				Themp::Print("Was unable to read the entire file [%S]", path.c_str());
				Themp::Break();
			}

			Themp::D3D::Material mat{};
			mat.ParseMaterialFile(materialData);
		}
	}
}