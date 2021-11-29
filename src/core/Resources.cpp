#include "Engine.h"
#include "Resources.h"
#include "renderer/control.h"
#include "renderer/gpu_resources.h"
#include "renderer/shadercompiler.h"
#include "util/print.h"
#include "util/break.h"
#include "util/stringUtils.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <charconv>
#include <optional>
#include <sstream>

#include "util/svars.h"

#define TOML_EXCEPTIONS 0
#include <toml.hpp>

namespace Themp
{
	std::unique_ptr<Resources> Resources::instance;

#define RESOURCES_FOLDER L"../resources/"
#define MATERIALS_FOLDER RESOURCES_FOLDER"materials/"
#define PASSES_FOLDER RESOURCES_FOLDER"passes/"
#define SHADERS_FOLDER RESOURCES_FOLDER"shaders/"
#define RENDER_TARGET_FOLDER RESOURCES_FOLDER"rendertargets/"


	// TODO fix string conversions
	//I'd like to start off mentioning I know that conversion from string -> wstring is done wrongly,
	//my first priority is getting it all up and running in a regular ascii environment, 
	//so handling utf8 and wchar conversions is atm out of my scope.

	D3D::Texture& Resources::Get(D3D::RTVHandle handle)
	{
		return m_ColorTargets[handle.handle].second;
	}
	D3D::Texture& Resources::Get(D3D::DSVHandle handle)
	{
		return m_DepthTargets[handle.handle].second;
	}
	D3D::Texture& Resources::Get(D3D::SRVHandle handle)
	{
		return m_SRVs[handle.handle].second;
	}
	D3D::SubPass& Resources::Get(D3D::SubPassHandle handle)
	{
		return m_Subpasses[handle.handle];
	}
	D3D::Pass& Resources::Get(D3D::PassHandle handle)
	{
		return m_Passes[handle.handle];
	}
	D3D::Shader& Resources::Get(D3D::ShaderHandle handle)
	{
		return m_Shaders[handle.handle];
	}

	std::wstring GetFilePath(std::wstring_view base, std::wstring_view filename, std::wstring_view extension)
	{
		std::wstring path = std::wstring(base);
		path.reserve(filename.size() + extension.size() + path.size());
		path.append(filename.begin(), filename.end())
			.append(extension.begin(), extension.end());

		return path;
	}


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

	std::vector<D3D::SubPass>& Resources::LoadMaterials()
	{
		std::vector<std::wstring> materials = LoadFilesFromDirectory(MATERIALS_FOLDER);
		std::vector<std::wstring> shaderFiles = LoadFilesFromDirectory(SHADERS_FOLDER);
		std::string materialData(10240, '\0');
		for (const std::wstring& path : materials)
		{
			Themp::Print("Loading material file: [%S]", path.c_str());
			auto passes = LoadMaterial(ReadFileToString(path), shaderFiles);
			MergePasses(passes);
		}

		const D3D::ShaderCompiler& compiler = Themp::Engine::instance->m_Renderer->GetShaderCompiler();
		for (int i = 0; i < m_Shaders.size(); i++)
		{
			compiler.Compile(m_Shaders[i]);
		}

		return m_Subpasses;
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

		const toml::parse_result result = toml::parse(data);
		if (!result)
		{
			Themp::Print("%*s at line [%i:%i]", result.error().description().size(), result.error().description().data(), result.error().source().begin.line, result.error().source().begin.column);
			Themp::Break();
			return PassHandle::Invalid;
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
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::ConservativeRaster)], pass, &Pass::SetConservativeRaster))
		{
			Themp::Print("ConservativeRaster was not of string type or not found");
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
		if (!SetPassMember(result[Pass::GetPassMemberAsString(PassMember::Topology)], pass, &Pass::SetTopology))
		{
			Themp::Print("Topology was not of string type or not found");
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
						pass.SetColorTarget(it.second.as_integer()->get(), LoadRenderTarget(it.first));
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
			pass.SetColorTarget(0, LoadRenderTarget(colorTargets.as_string()->get()));
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

	D3D::ShaderHandle Resources::LoadShader(std::string_view filename, std::vector<std::wstring>& shaderFiles)
	{
		for (int i = 0; i < m_Shaders.size(); i++)
		{
			if (m_Shaders[i].GetName() == filename)
			{
				Themp::Print("Found Shader: [%*s] in cache!", filename.size(), filename.data());
				return i;
			}
		}

		std::wstring name = std::wstring(filename.begin(), filename.end());

		static const std::unordered_map<std::wstring_view, D3D::Shader::ShaderType> TargetToType =
		{
			{L"_vs", D3D::Shader::ShaderType::Vertex},
			{L"_ps", D3D::Shader::ShaderType::Pixel},
			{L"_gs", D3D::Shader::ShaderType::Geometry},
			{L"_ds", D3D::Shader::ShaderType::Domain},
			{L"_hs", D3D::Shader::ShaderType::Hull},
			{L"_cs", D3D::Shader::ShaderType::Compute},
			{L"_ms", D3D::Shader::ShaderType::Mesh},
			{L"_as", D3D::Shader::ShaderType::Amplify},
			{L"_rh", D3D::Shader::ShaderType::RayHit},
			{L"_rm", D3D::Shader::ShaderType::RayMiss},
		};

		D3D::Shader shader{};
		shader.Init(std::string(filename));

		for (int i = 0; i < shaderFiles.size(); i++)
		{
			std::wstring_view fileExt;
			const auto& lastBackSlash = shaderFiles[i].find_last_of(L'\\');
			const auto& lastForwardSlash = shaderFiles[i].find_last_of(L'/');
			if (lastBackSlash != std::wstring::npos)
			{
				fileExt = std::wstring_view(shaderFiles[i].data() + lastBackSlash + 1);
			} 
			else if (lastForwardSlash != std::wstring::npos)
			{
				fileExt = std::wstring_view(shaderFiles[i].data() + lastForwardSlash + 1);
			}
			else
			{
				continue;
			}


			const auto& period = fileExt.find_last_of(L'.');
			const auto& underscore = fileExt.find_last_of(L'_');

			//not great, there's still some edge cases but it'll do if we just all keep to conventions :P
			if (underscore == std::wstring::npos || period == std::wstring::npos || underscore > period)
			{
				continue;
			}

			std::wstring_view target = std::wstring_view(fileExt.data() + underscore, period - underscore);
			std::wstring_view fileNoExt = std::wstring_view(fileExt.data(), underscore);

			if (name.size() == fileNoExt.size() && fileNoExt == name)
			{
				const auto& targetIt = TargetToType.find(target);
				if (targetIt != TargetToType.end())
				{
					shader.AddShaderSource(name, targetIt->second);
				}
			}		
		}
		if (shader.IsValid())
		{
			m_Shaders.push_back(shader);
			return m_Shaders.size() - 1;
		}

		Themp::Print("No shaders with leading name: [%*s] were found!", filename.size(), filename.data());
		Themp::Break();

		return D3D::ShaderHandle::Invalid;
	}

	D3D::RenderTargetHandle Resources::LoadRenderTarget(std::string_view filename)
	{
		D3D::RTVHandle rtvHandle = D3D::RTVHandle::Invalid;
		D3D::DSVHandle dsvHandle = D3D::DSVHandle::Invalid;
		D3D::SRVHandle srvHandle = D3D::DSVHandle::Invalid;


		for (int i = 0; i < m_ColorTargets.size(); i++)
		{
			if (m_ColorTargets[i].first == filename)
			{
				rtvHandle = (D3D::RTVHandle)i;
				break;
			}
		}

		for (int i = 0; i < m_DepthTargets.size(); i++)
		{
			if (m_DepthTargets[i].first == filename)
			{
				dsvHandle = (D3D::DSVHandle)i;
				break;
			}
		}

		for (int i = 0; i < m_SRVs.size(); i++)
		{
			if (m_SRVs[i].first == filename)
			{
				srvHandle = (D3D::SRVHandle)i;
				break;
			}
		}
		if (rtvHandle.IsValid() || dsvHandle.IsValid() || srvHandle.IsValid())
		{
			Themp::Print("Found Rendertarget: [%*s] in cache!", filename.size(), filename.data());
			return { rtvHandle, dsvHandle, srvHandle };
		}



		std::string data = ReadFileToString(GetFilePath(RENDER_TARGET_FOLDER, std::wstring(filename.begin(),filename.end()), L".target"));
		const toml::parse_result result = toml::parse(Themp::Util::ToLowerCase(data));
		if (!result)
		{
			Themp::Print("%*s at line [%i:%i]", result.error().description().size(), result.error().description().data(), result.error().source().begin.line, result.error().source().begin.column);
			Themp::Break();
			return D3D::RenderTargetHandle::Invalid();
		}

		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;
		DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
		DXGI_SAMPLE_DESC multisample{};
		int width = 0, height = 0, depth = 0;
		D3D12_CLEAR_VALUE clearValue{};
		D3D::TEXTURE_TYPE textureType = D3D::TEXTURE_TYPE::RTV;
		bool createSRV = false;
		bool doesScale = true;
		float scale = 1.0f;

		//past tense
		const auto& readType = result["type"];
		if (readType && readType.is_string())
		{
			std::string val = readType.as_string()->get();
			if (val == "color")
			{
				flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			}
			else if (val == "depth")
			{
				flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			}
		}
		if (flags == D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE)
		{
			Themp::Print("Type was not of type string or not found!");
			Themp::Break();
		}

		const auto& makeSRV = result["createshaderview"];
		if (makeSRV && makeSRV.is_boolean())
		{
			createSRV = makeSRV.as_boolean()->get();
		}

		const auto& readFormat = result["format"];
		if (readFormat && readFormat.is_string())
		{
			D3D::TextureFormat formatFromStr{};
			formatFromStr.SetFromString(readFormat.as_string()->get());
			format = D3D::DxTranslator::GetTextureFormat(formatFromStr);
			clearValue.Format = format;
		}

		const auto& readWidth = result["fixedwidth"];
		if (readWidth && readWidth.is_integer())
		{
			width = readWidth.as_integer()->get();
		}

		const auto& readHeight = result["fixedheight"];
		if (readHeight && readHeight.is_integer())
		{
			height = readHeight.as_integer()->get();
		}

		const auto& readCount = result["multisamplecount"];
		if (readCount && readCount.is_integer())
		{
			multisample.Count = readCount.as_integer()->get();
		}

		const auto& readQuality = result["multisamplequality"];
		if (readQuality && readQuality.is_integer())
		{
			multisample.Quality = readQuality.as_integer()->get();
		}

		const auto& resolution = result["resolution"];
		if (resolution && resolution.is_string())
		{
			doesScale = resolution.as_string()->get() != "fixed";
		}

		const auto& readScale = result["scale"];
		if (readScale && readScale.is_floating_point())
		{
			scale = readScale.as_floating_point()->get();
		}

		if (doesScale)
		{
			int resX = Engine::s_SVars.GetSVarInt(SVar::iWindowWidth);
			int resY = Engine::s_SVars.GetSVarInt(SVar::iWindowHeight);
			width = static_cast<int>(static_cast<float>(resX) * scale);
			height = static_cast<int>(static_cast<float>(resY) * scale);
		}

		auto& resourceManager = Engine::instance->m_Renderer->GetResourceManager();
		auto device = Engine::instance->m_Renderer->GetDevice();
		auto resource = resourceManager.GetTextureResource(device, std::wstring(filename.begin(),filename.end()), flags, format, 1, multisample, width, height, depth, &clearValue, textureType);
		auto& tex = resourceManager.GetTextureFromResource(device, resource, textureType);

		switch (textureType)
		{
		case D3D::TEXTURE_TYPE::RTV:
			m_ColorTargets.push_back({ std::string(filename), tex });
			rtvHandle = m_ColorTargets.size() - 1;
			break;
		case D3D::TEXTURE_TYPE::DSV:
			m_DepthTargets.push_back({ std::string(filename), tex });
			dsvHandle = m_DepthTargets.size() - 1;
			break;
		}

		if (createSRV)
		{
			auto& srv = resourceManager.GetTextureFromResource(device, resource, D3D::TEXTURE_TYPE::SRV);
			m_SRVs.push_back({ std::string(filename), srv });
			srvHandle = m_SRVs.size() - 1;
		}
		
		return {rtvHandle, dsvHandle, srvHandle};
	}
	
	void Resources::MergePasses(std::vector<D3D::SubPass> passes)
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

	std::vector<D3D::SubPass> Resources::LoadMaterial(const std::string& data, std::vector<std::wstring>& shaderFiles)
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

		std::vector<D3D::SubPass> passData;

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
					ShaderHandle shaderHandle = LoadShader(shader.as_string()->get(), shaderFiles);

					passData.push_back({ passHandle , shaderHandle });
				}
			}
		}
		return passData;

	}

}