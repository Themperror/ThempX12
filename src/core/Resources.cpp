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


#define TOML_EXCEPTIONS 0
#include <toml.hpp>

namespace Themp
{
	std::unique_ptr<Resources> Resources::instance;

#define RESOURCES_FOLDER L"../resources/"
#define MATERIALS_FOLDER RESOURCES_FOLDER"materials/"
#define PASSES_FOLDER RESOURCES_FOLDER"passes/"
#define SHADERS_FOLDER RESOURCES_FOLDER"shaders/"


	std::vector<std::wstring> LoadFilesFromDirectory(std::wstring dir)
	{
		std::vector<std::wstring> files;

		DWORD attributes = GetFileAttributesW(dir.c_str());
		if (attributes == INVALID_FILE_ATTRIBUTES)
		{
			Themp::Print(L"Unable to find folder at " MATERIALS_FOLDER);
			Themp::Break();
			return {};
		}

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
		std::string data(fileSize, '\0');
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
		CloseHandle(file);

		return data;
	}

	void Resources::LoadMaterials()
	{
		std::vector<std::wstring> materials = LoadFilesFromDirectory(MATERIALS_FOLDER);

		std::string materialData(10240, '\0');
		for (const std::wstring& path : materials)
		{
			Themp::Print("Loading material file: [%S]", path.c_str());
			auto passes = LoadMaterial(ReadFileToString(path));
			MergePasses(passes);
		}
	}


	void Resources::AddOrSetShaderToPass(std::string_view name, int index, Themp::D3D::Pass& pass)
	{

	}

	template<typename T>
	bool SetPassMember(const toml::node_view<const toml::node>& node, Themp::D3D::Pass& pass, void (Themp::D3D::Pass::* func)(T))
	{
		if constexpr (std::is_same_v<T, int8_t> ||  std::is_same_v<T, int32_t> || std::is_same_v<T, int64_t> || std::is_same_v<T, uint8_t> || std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>)
		{
			const auto& t = node.as<int64_t>();
			if (t)
			{
				(pass.*func)((T)t->get());
				return true;
			}
		}
		else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>)
		{
			const auto& t = node.as<double>();
			if (t)
			{
				(pass.*func)((T)t->get());
				return true;
			}
		}else if constexpr (std::is_same_v<T, std::string_view> || std::is_same_v<T, std::string>)
		{
			const auto& t = node.as<std::string>();
			if (t)
			{
				(pass.*func)(T{ t->get() });
				return true;
			}
		}
		else
		{
			const auto& t = node.as<T>();
			if (t)
			{
				(pass.*func)((T)t->get());
				return true;
			}
		}

		return false;
	}


	Themp::D3D::PassHandle Resources::LoadPass(std::string_view filename)
	{
		for (int i = 0; i < m_Passes.size(); i++)
		{
			if (m_Passes[i].GetName() == filename)
			{
				Themp::Print("Found Pass: [%*s] in cache!", filename.size(), filename.data());
				return static_cast<Themp::D3D::PassHandle>(i);
			}
		}
		using namespace Themp::D3D;

		Pass pass{ filename };

		//std::format is C++20 :(
		const wchar_t* extension = L".pass";
		std::wstring path = std::wstring(PASSES_FOLDER);
		path.reserve(filename.size() + wcslen(extension) + path.size());
		path.append(filename.begin(), filename.end())
			.append(extension);

		std::string data = Themp::Util::ToLowerCase(ReadFileToString(path));

		Themp::Print("Parsing %S", path.c_str());
		const toml::parse_result result = toml::parse(data);
		if (!result)
		{
			Themp::Print("%*s at line [%i:%i]", result.error().description().size(), result.error().description().data(), result.error().source().begin.line, result.error().source().begin.column);
			Themp::Break();
		}


		using PassMember = Pass::Member;

		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::Priority)], pass, &Pass::SetPriority))
		{
			Themp::Print("priority was not of integer type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::DepthEnable)], pass, &Pass::SetDepthEnable))
		{
			Themp::Print("depthenable was not of boolean type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::DepthWriteMask)], pass, &Pass::SetDepthWriteMask))
		{
			Themp::Print("DepthWriteMask was not of boolean type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::DepthFunc)], pass, &Pass::SetDepthFunc))
		{
			Themp::Print("DepthFunc was not of string type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::StencilEnable)], pass, &Pass::SetStencilEnable))
		{
			Themp::Print("StencilEnable was not of int type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::StencilReadMask)], pass, &Pass::SetStencilReadMask))
		{
			Themp::Print("StencilReadMask was not of int type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::StencilWriteMask)], pass, &Pass::SetStencilWriteMask))
		{
			Themp::Print("StencilWriteMask was not of string type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::FrontFaceFailOp)], pass, &Pass::SetFrontFaceFailOp))
		{
			Themp::Print("FrontFaceFailOp was not of string type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::FrontFaceDepthFailOp)], pass, &Pass::SetFrontFaceDepthFailOp))
		{
			Themp::Print("FrontFaceDepthFailOp was not of string type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::FrontFacePassOp)], pass, &Pass::SetFrontFacePassOp))
		{
			Themp::Print("FrontFacePassOp was not of string type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::FrontFaceFunc)], pass, &Pass::SetFrontFaceFunc))
		{
			Themp::Print("FrontFaceFunc was not of string type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::BackFaceFailOp)], pass, &Pass::SetBackFaceFailOp))
		{
			Themp::Print("BackFaceFailOp was not of string type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::BackFaceDepthFailOp)], pass, &Pass::SetBackFaceDepthFailOp))
		{
			Themp::Print("BackFaceDepthFailOp was not of string type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::BackFacePassOp)], pass, &Pass::SetBackFacePassOp))
		{
			Themp::Print("BackFacePassOp was not of string type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::BackFaceFunc)], pass, &Pass::SetBackFaceFunc))
		{
			Themp::Print("BackFaceFunc was not of string type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::FrontCounterClockwise)], pass, &Pass::SetFrontCounterClockwise))
		{
			Themp::Print("FrontCounterClockwise was not of bool type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::FillMode)], pass, &Pass::SetFillMode))
		{
			Themp::Print("FillMode was not of string type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::CullMode)], pass, &Pass::SetCullMode))
		{
			Themp::Print("CullMode was not of string type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::DepthBias)], pass, &Pass::SetDepthBias))
		{
			Themp::Print("DepthBias was not of int type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::DepthBiasClamp)], pass, &Pass::SetDepthBiasClamp))
		{
			Themp::Print("DepthBiasClamp was not of float type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::SlopeScaledDepthBias)], pass, &Pass::SetSlopeScaledDepthBias))
		{
			Themp::Print("SlopeScaledDepthBias was not of float type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::DepthClipEnable)], pass, &Pass::SetDepthClipEnable))
		{
			Themp::Print("DepthClipEnable was not of bool type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::MultisampleEnable)], pass, &Pass::SetMultisampleEnable))
		{
			Themp::Print("MultisampleEnable was not of bool type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::AntialiasedLineEnable)], pass, &Pass::SetAntialiasedLineEnable))
		{
			Themp::Print("AntialiasedLineEnable was not of bool type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::ForcedSampleCount)], pass, &Pass::SetForcedSampleCount))
		{
			Themp::Print("ForcedSampleCount was not of int type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::AlphaToCoverage)], pass, &Pass::SetAlphaToCoverageEnable))
		{
			Themp::Print("AlphaToCoverage was not of bool type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::IndependendBlend)], pass, &Pass::SetIndependendBlendEnable))
		{
			Themp::Print("IndependendBlend was not of bool type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::SampleMask)], pass, &Pass::SetSampleMask))
		{
			Themp::Print("SampleMask was not of int type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::MultisampleCount)], pass, &Pass::SetMultisampleCount))
		{
			Themp::Print("MultisampleCount was not of int type or not found");
		}
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::MultisampleQuality)], pass, &Pass::SetMultisampleQuality))
		{
			Themp::Print("MultisampleQuality was not of int type or not found");
		}


		const auto& colorTargets = result[Pass::GetPassMemberAsString(PassMember::ColorTarget)];
		if (colorTargets.is_array()) //array of targets with specific index
		{
			const auto& arr = colorTargets.as_array();
			for (const auto& it : *arr)
			{
				const auto& targetTable = it.as_table();
				for (const auto& it : *targetTable)
				{
					if (it.second.is_integer())
					{
						pass.SetColorTarget(LoadRenderTarget(it.first), it.second.as_integer()->get());
					}
					else
					{
						Themp::Print("paired value with %s was not of integer type or not found!", it.first.c_str());
					}
				}
			}
		}
		else if (colorTargets.is_string()) //allow single target to force slot 0
		{
			pass.SetColorTarget(LoadRenderTarget(colorTargets.as_string()->get()), 0);
		}

		const auto& depthTarget = result[Pass::GetPassMemberAsString(PassMember::DepthTarget)];
		if (depthTarget.is_string()) //allow single target to force slot 0
		{
			pass.SetDepthTarget(LoadRenderTarget(depthTarget.as_string()->get()));
		}

		const auto& blendStates = result[Pass::GetPassMemberAsString(PassMember::BlendTargets)];
		if (blendStates.is_array()) //array of targets with specific index
		{
			const auto& arr = blendStates.as_array();
			for (const auto& it : *arr)
			{
				const auto& blendState = *it.as_table();
				Themp::D3D::Pass::BlendState state{};
				const auto& rti = blendState[Pass::GetPassMemberAsString(PassMember::RenderTargetIndex)];
				int index = 0;
				if (rti && rti.is_integer())
				{
					index = rti.as_integer()->get();
				}
				else
				{
					Themp::Print("No RenderTargetIndex supplied for blendstate!");
				}

				const auto& blendEnable = blendState[Pass::GetPassMemberAsString(PassMember::BlendEnable)];
				if (blendEnable && blendEnable.is_boolean())
				{
					state.blendEnable = blendEnable.as_boolean()->get();
				}
				const auto& logicOpEnable = blendState[Pass::GetPassMemberAsString(PassMember::LogicOpEnable)];
				if (logicOpEnable && logicOpEnable.is_boolean())
				{
					state.logicOpEnable = logicOpEnable.as_boolean()->get();
				}

				const auto& blendOp = blendState[Pass::GetPassMemberAsString(PassMember::BlendOp)];
				if (blendOp && blendOp.is_string())
				{
					state.blendOp.SetFromString(blendOp.as_string()->get());
				}
				const auto& blendAlphaOp = blendState[Pass::GetPassMemberAsString(PassMember::BlendAlphaOp)];
				if (blendAlphaOp && blendAlphaOp.is_string())
				{
					state.blendAlphaOp.SetFromString(blendAlphaOp.as_string()->get());
				}

				const auto& srcBlend = blendState[Pass::GetPassMemberAsString(PassMember::SrcBlend)];
				if (srcBlend && srcBlend.is_string())
				{
					state.srcBlend.SetFromString(srcBlend.as_string()->get());
				}
				const auto& destBlend = blendState[Pass::GetPassMemberAsString(PassMember::DestBlend)];
				if (destBlend && destBlend.is_string())
				{
					state.destBlend.SetFromString(destBlend.as_string()->get());
				}

				const auto& srcBlendAlpha = blendState[Pass::GetPassMemberAsString(PassMember::SrcAlphaBlend)];
				if (srcBlendAlpha && srcBlendAlpha.is_string())
				{
					state.srcBlendAlpha.SetFromString(srcBlendAlpha.as_string()->get());
				}
				const auto& destBlendAlpha = blendState[Pass::GetPassMemberAsString(PassMember::DestAlphaBlend)];
				if (destBlendAlpha && destBlendAlpha.is_string())
				{
					state.destBlendAlpha.SetFromString(destBlendAlpha.as_string()->get());
				}

				const auto& logicOp = blendState[Pass::GetPassMemberAsString(PassMember::LogicOp)];
				if (logicOp && logicOp.is_string())
				{
					state.logicOp.SetFromString(logicOp.as_string()->get());
				}

				const auto& renderTargetWriteMask = blendState[Pass::GetPassMemberAsString(PassMember::RenderTargetWriteMask)];
				if (renderTargetWriteMask && renderTargetWriteMask.is_integer())
				{
					state.renderTargetWriteMask = renderTargetWriteMask.as_integer()->get();
				}
				
				pass.SetBlendTarget(index, state);
			}
		}
		
		m_Passes.push_back(pass);

		return m_Passes.size()-1;
	}

	D3D::ShaderHandle Resources::LoadShader(std::string_view filename)
	{
		return 0;
	}

	D3D::RenderTargetHandle Resources::LoadRenderTarget(std::string_view filename)
	{
		return 0;
	}
	
	void Resources::MergePasses(std::vector<Resources::SubPass> passes)
	{
		for (int j = 0; j < passes.size(); j++)
		{
			bool foundPassWithShader = false;
			for (int i = 0; i < m_Subpasses.size(); i++)
			{
				if (passes[j].pass == m_Subpasses[i].pass && passes[j].shader == m_Subpasses[i].shader)
				{
					foundPassWithShader = true;
				}
			}
			if (!foundPassWithShader)
			{
				m_Subpasses.push_back(passes[j]);
			}
		}
	}

	std::vector<Resources::SubPass> Resources::LoadMaterial(const std::string& data)
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

		const toml::parse_result result = toml::parse(Themp::Util::ToLowerCase(data));
		if (!result)
		{
			Themp::Print("%*s at line [%i:%i]", result.error().description().size(), result.error().description().data(), result.error().source().begin.line, result.error().source().begin.column);
			Themp::Break();
		}

		std::vector<Resources::SubPass> passData;

		const auto& passes = result["subpass"];
		if (passes.is_array_of_tables())
		{
			const auto& arr = passes.as_array();
			for (const auto& it : *arr)
			{
				const auto& table = *it.as_table();
				const auto& pass = table[validEntries[PASS]];
				const auto& shader = table[validEntries[SHADER]];
				if (pass && shader)
				{
					PassHandle passHandle = LoadPass(pass.as_string()->get());
					ShaderHandle shaderHandle = LoadShader(shader.as_string()->get());
					passData.push_back({ passHandle , shaderHandle });
				}
			}
		}
		return passData;

	}

}