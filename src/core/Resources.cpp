#include "Resources.h"
#include "Engine.h"
#include "renderer/control.h"
#include "renderer/gpu_resources.h"
#include "renderer/shadercompiler.h"
#include "core/scripting/asengine.h"
#include "renderer/material.h"
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
#include <lib/toml.hpp>

namespace Themp
{
	std::unique_ptr<Resources> Resources::instance;

#define RESOURCES_FOLDER "../resources/"
#define MATERIALS_FOLDER RESOURCES_FOLDER"materials/"
#define MODELS_FOLDER RESOURCES_FOLDER"models/"
#define SCENES_FOLDER RESOURCES_FOLDER"scenes/"
#define SCRIPTS_FOLDER RESOURCES_FOLDER"scripts/"
#define PASSES_FOLDER RESOURCES_FOLDER"passes/"
#define SHADERS_FOLDER RESOURCES_FOLDER"shaders/"
#define RENDER_TARGET_FOLDER RESOURCES_FOLDER"rendertargets/"

	std::string_view Resources::GetResourcesFolder()
	{
		return RESOURCES_FOLDER;
	}
	std::string_view Resources::GetMaterialsFolder()
	{
		return MATERIALS_FOLDER;
	}
	std::string_view Resources::GetModelsFolder()
	{
		return MODELS_FOLDER;
	}
	std::string_view Resources::GetScenesFolder()
	{
		return SCENES_FOLDER;
	}
	std::string_view Resources::GetScriptsFolder()
	{
		return SCRIPTS_FOLDER;
	}
	std::string_view Resources::GetPassesFolder()
	{
		return PASSES_FOLDER;
	}
	std::string_view Resources::GetShadersFolder()
	{
		return SHADERS_FOLDER;
	}
	std::string_view Resources::GetRenderTargetsFolder()
	{
		return RENDER_TARGET_FOLDER;
	}

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
	D3D::Material& Resources::Get(D3D::MaterialHandle handle)
	{
		return m_Materials[handle.handle];
	}

	std::string GetFilePath(std::string_view base, std::string_view filename, std::string_view extension)
	{
		std::string path = std::string(base);
		path.reserve(filename.size() + extension.size() + path.size());
		path.append(filename.begin(), filename.end())
			.append(extension.begin(), extension.end());

		return path;
	}

	std::string GetFileName(std::string_view path)
	{
		std::string filename = std::string(path.begin(), path.end());
		for (size_t i = 0; i < filename.size(); i++)
		{
			if (filename[i] == '/')
			{
				filename[i] = '\\';
			}
		}
		size_t lastBackSlash = filename.find_last_of('\\');
		if (lastBackSlash != std::string::npos && lastBackSlash +1 < filename.size())
		{
			filename = filename.substr(lastBackSlash + 1, filename.size() - lastBackSlash - 1);
		}

		size_t lastPeriod = filename.find_last_of('.');
		if (lastPeriod != std::string::npos)
		{
			filename = filename.substr(0, lastPeriod);
		}

		return filename;
	}

	std::vector<std::string> LoadFilesFromDirectory(std::string dir)
	{
		std::vector<std::string> files;

		DWORD attributes = GetFileAttributesA(dir.c_str());
		if (attributes == INVALID_FILE_ATTRIBUTES)
		{
			Themp::Print(L"Unable to find folder at %s", dir.c_str());
			Themp::Break();
			return {};
		}

		WIN32_FIND_DATA ffd{};
		HANDLE hFind = FindFirstFileA((dir + "*").c_str(), &ffd);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			Themp::Print("Something went wrong %s", dir.c_str());
		}
		while (FindNextFileA(hFind, &ffd) != 0)
		{
			// ignore directories
			if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				// create a full path for each file we find, e.g. "c:\indir\foo.txt"
				std::string file_path;
				file_path.reserve(2048);
				file_path = dir + ffd.cFileName;
				std::transform(file_path.begin(), file_path.end(), file_path.begin(), ::tolower);
				files.push_back(file_path);
			}
		}
		FindClose(hFind);
		return files;
	}

	std::vector<std::pair<std::string, std::string>> Resources::GetScriptFiles()
	{
		auto files = LoadFilesFromDirectory(SCRIPTS_FOLDER);
		std::vector<std::pair<std::string, std::string>> fileDatas;
		fileDatas.reserve(files.size());
		for (auto& file : files)
		{
			fileDatas.push_back({ file, Engine::ReadFileToString(file)});
		}
		return fileDatas;
	}


	void Resources::ResizeRendertargets(int width, int height)
	{
		std::vector<std::pair<int,int>> releasedIndices;
		auto& gpuResources = Engine::instance->m_Renderer->GetResourceManager();
		auto device = Engine::instance->m_Renderer->GetDevice();
		for (int i = 1; i < m_ColorTargets.size(); i++)
		{
			auto desc = m_ColorTargets[i].second.GetResource(D3D::TEXTURE_TYPE::RTV)->GetDesc();
			int releasedIndex = gpuResources.ReleaseTexture(m_ColorTargets[i].second);
			if (m_ColorTargets[i].first.doesScale)
			{
				float scale = m_ColorTargets[i].first.scalar;
				int resX = Engine::s_SVars.GetSVarInt(SVar::iWindowWidth);
				int resY = Engine::s_SVars.GetSVarInt(SVar::iWindowHeight);
				desc.Width = static_cast<UINT64>(static_cast<float>(resX) * scale);
				desc.Height = static_cast<UINT>(static_cast<float>(resY) * scale);
			}

			D3D::TEXTURE_TYPE outType;
			auto newResource = gpuResources.GetTextureResource(device, m_ColorTargets[i].first.name, desc.Flags, desc.Format, desc.MipLevels, desc.SampleDesc, desc.Width, desc.Height, 0, &m_ColorTargets[i].first.clearValue, outType);
			m_ColorTargets[i].second = gpuResources.GetTextureFromResource(device, newResource, outType, releasedIndex);
		}
		for (int i = 0; i < m_DepthTargets.size(); i++)
		{
			auto desc = m_DepthTargets[i].second.GetResource(D3D::TEXTURE_TYPE::DSV)->GetDesc();
			int releasedIndex = gpuResources.ReleaseTexture(m_DepthTargets[i].second);
			if (m_DepthTargets[i].first.doesScale)
			{
				float scale = m_DepthTargets[i].first.scalar;
				int resX = Engine::s_SVars.GetSVarInt(SVar::iWindowWidth);
				int resY = Engine::s_SVars.GetSVarInt(SVar::iWindowHeight);
				desc.Width = static_cast<UINT64>(static_cast<float>(resX) * scale);
				desc.Height = static_cast<UINT>(static_cast<float>(resY) * scale);
			}

			D3D::TEXTURE_TYPE outType;
			auto newResource = gpuResources.GetTextureResource(device, m_DepthTargets[i].first.name, desc.Flags, desc.Format, desc.MipLevels, desc.SampleDesc, desc.Width, desc.Height, 0, &m_DepthTargets[i].first.clearValue, outType);
			m_DepthTargets[i].second = gpuResources.GetTextureFromResource(device, newResource, outType, releasedIndex);
		}
	}
	void Resources::LoadMaterials()
	{
		std::vector<std::string> materials = LoadFilesFromDirectory(MATERIALS_FOLDER);
		std::vector<std::string> shaderFiles = LoadFilesFromDirectory(SHADERS_FOLDER);
		std::string materialData(10240, '\0');
		std::vector<D3D::SubPass> passes;
		for (const std::string& path : materials)
		{
			std::string fileName = GetFileName(path);
			D3D::Material material;
			for (int i = 0; i < m_Materials.size(); i++)
			{
				if (m_Materials[i].m_Name == fileName)
				{
					goto CONTINUE;
				}
			}
			Themp::Print("Loading material file: [%s]", path.c_str());
			passes.clear();
			passes = LoadMaterial(Engine::ReadFileToString(path), shaderFiles);

			MergePasses(passes);
			for (int i = 0; i < m_Subpasses.size(); i++)
			{
				for (int j = 0; j < passes.size(); j++)
				{
					if (passes[j].pass == m_Subpasses[i].pass)
					{
						material.m_SubPasses.push_back(i);
					}
				}
			}
			material.m_Name = GetFileName(path);
			m_Materials.push_back(material);

		CONTINUE:
			continue;
		}

		const D3D::ShaderCompiler& compiler = Themp::Engine::instance->m_Renderer->GetShaderCompiler();
		for (int i = 0; i < m_Shaders.size(); i++)
		{
			compiler.Compile(m_Shaders[i]);
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
		const char* extension = ".pass";
		std::string path = std::string(PASSES_FOLDER);
		path.reserve(filename.size() + strlen(extension) + path.size());
		path.append(filename.begin(), filename.end())
			.append(extension);

		std::string data = Themp::Util::ToLowerCase(Engine::ReadFileToString(path));

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
						pass.SetColorTarget(static_cast<int>(it.second.as_integer()->get()), LoadRenderTarget(it.first));
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
					index = static_cast<int>(rti.as_integer()->get());
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
					int64_t val = renderTargetWriteMask.as_integer()->get();
					if (val > 255 || val < 0)
					{
						Themp::Print("RenderTargetWriteMask expected an UINT8, not %lli!", val);
					}
					state.renderTargetWriteMask = static_cast<uint8_t>(val);
				}
				
				pass.SetBlendTarget(index, state);
			}
		}
		

		const auto& viewports = result[Pass::GetPassMemberAsString(PassMember::Viewports)];
		if (viewports.is_array()) //array of targets with specific index
		{
			const auto& arr = viewports.as_array();
			for (const auto& it : *arr)
			{
				const auto& viewportTable = *it.as_table();
				Pass::Viewport viewport{};
				const auto& rti = viewportTable[Pass::GetPassMemberAsString(PassMember::RenderTargetIndex)];
				int index = 0;
				if (rti && rti.is_integer())
				{
					index = static_cast<int>(rti.as_integer()->get());
				}
				else
				{
					Themp::Print("No RenderTargetIndex supplied (or not integer) for viewport!");
				}

				const auto& fixedRes = viewportTable[Pass::GetPassMemberAsString(PassMember::FixedResolution)];
				if (fixedRes && fixedRes.is_boolean())
				{
					viewport.fixedResolution = fixedRes.as_boolean()->get();
				}
				else
				{
					Themp::Print("No FixedResolution (or not bool) supplied for viewport!");
				}
				const auto& scaler = viewportTable[Pass::GetPassMemberAsString(PassMember::Scaler)];
				if (scaler && scaler.is_floating_point())
				{
					viewport.scaler = static_cast<float>(scaler.as_floating_point()->get());
				}
				else
				{
					Themp::Print("No Scaler supplied (or not float) for viewport!");
				}
				const auto& topLeftX = viewportTable[Pass::GetPassMemberAsString(PassMember::VP_TopLeftX)];
				if (topLeftX && topLeftX.is_floating_point())
				{
					viewport.topLeftX = static_cast<float>(topLeftX.as_floating_point()->get());
				}
				else
				{
					Themp::Print("No TopLeftX supplied (or not float) for viewport!");
				}
				const auto& topLeftY = viewportTable[Pass::GetPassMemberAsString(PassMember::VP_TopLeftY)];
				if (topLeftY && topLeftY.is_floating_point())
				{
					viewport.topLeftY = static_cast<float>(topLeftY.as_floating_point()->get());
				}
				else
				{
					Themp::Print("No TopLeftY supplied (or not float) for viewport!");
				}
				const auto& width = viewportTable[Pass::GetPassMemberAsString(PassMember::VP_Width)];
				if (width && width.is_floating_point())
				{
					viewport.width = static_cast<float>(width.as_floating_point()->get());
				}
				else
				{
					Themp::Print("No Width supplied (or not float) for viewport!");
				}
				const auto& height = viewportTable[Pass::GetPassMemberAsString(PassMember::VP_Height)];
				if (height && height.is_floating_point())
				{
					viewport.height = static_cast<float>(height.as_floating_point()->get());
				}
				else
				{
					Themp::Print("No Height supplied (or not float) for viewport!");
				}
				const auto& minDepth = viewportTable[Pass::GetPassMemberAsString(PassMember::VP_MinDepth)];
				if (minDepth && minDepth.is_floating_point())
				{
					viewport.minDepth = static_cast<float>(minDepth.as_floating_point()->get());
				}
				else
				{
					Themp::Print("No MinDepth supplied (or not float) for viewport!");
				}
				const auto& maxDepth = viewportTable[Pass::GetPassMemberAsString(PassMember::VP_MaxDepth)];
				if (maxDepth && maxDepth.is_floating_point())
				{
					viewport.maxDepth = static_cast<float>(maxDepth.as_floating_point()->get());
				}
				else
				{
					Themp::Print("No MaxDepth supplied (or not float) for viewport!");
				}

				pass.SetViewport(index, viewport);
			}
		}
		const auto& scissors = result[Pass::GetPassMemberAsString(PassMember::Scissors)];
		if (scissors.is_array()) //array of targets with specific index
		{
			const auto& arr = scissors.as_array();
			for (const auto& it : *arr)
			{
				const auto& scissorTable = *it.as_table();
				Pass::Scissor scissor{};
				const auto& rti = scissorTable[Pass::GetPassMemberAsString(PassMember::RenderTargetIndex)];
				int index = 0;
				if (rti && rti.is_integer())
				{
					index = static_cast<int>(rti.as_integer()->get());
				}
				else
				{
					Themp::Print("No RenderTargetIndex supplied (or not integer) for scissor!");
				}

				const auto& fixedRes = scissorTable[Pass::GetPassMemberAsString(PassMember::FixedResolution)];
				if (fixedRes && fixedRes.is_boolean())
				{
					scissor.fixedResolution = fixedRes.as_boolean()->get();
				}
				else
				{
					Themp::Print("No FixedResolution supplied (or not bool) for scissor!");
				}
				const auto& scaler = scissorTable[Pass::GetPassMemberAsString(PassMember::Scaler)];
				if (scaler && scaler.is_floating_point())
				{
					scissor.scaler = static_cast<float>(scaler.as_floating_point()->get());
				}
				else
				{
					Themp::Print("No Scaler supplied (or not integer) for scissor!");
				}
				const auto& left = scissorTable[Pass::GetPassMemberAsString(PassMember::Scissor_Left)];
				if (left && left.is_integer())
				{
					scissor.left = static_cast<uint32_t>(left.as_integer()->get());
				}
				else
				{
					Themp::Print("No Left supplied (or not integer) for scissor!");
				}
				const auto& right = scissorTable[Pass::GetPassMemberAsString(PassMember::Scissor_Right)];
				if (right && right.is_integer())
				{
					scissor.right = static_cast<uint32_t>(right.as_integer()->get());
				}
				else
				{
					Themp::Print("No Right supplied (or not integer) for scissor!");
				}
				const auto& top = scissorTable[Pass::GetPassMemberAsString(PassMember::Scissor_Top)];
				if (top && top.is_integer())
				{
					scissor.top = static_cast<uint32_t>(top.as_integer()->get());
				}
				else
				{
					Themp::Print("No Top supplied (or not integer) for scissor!");
				}
				const auto& bottom = scissorTable[Pass::GetPassMemberAsString(PassMember::Scissor_Bottom)];
				if (bottom && bottom.is_integer())
				{
					scissor.bottom = static_cast<uint32_t>(bottom.as_integer()->get());
				}
				else
				{
					Themp::Print("No Bottom supplied (or not integer) for scissor!");
				}

				pass.SetScissor(index, scissor);
			}
		}



		m_Passes.push_back(pass);

		return m_Passes.size()-1;
	}

	D3D::ShaderHandle Resources::LoadShader(std::string_view filename, std::vector<std::string>& shaderFiles)
	{
		for (int i = 0; i < m_Shaders.size(); i++)
		{
			if (m_Shaders[i].GetName() == filename)
			{
				Themp::Print("Found Shader: [%*s] in cache!", filename.size(), filename.data());
				return i;
			}
		}

		std::string name = std::string(filename.begin(), filename.end());

		static const std::unordered_map<std::string_view, D3D::Shader::ShaderType> TargetToType =
		{
			{"_vs", D3D::Shader::ShaderType::Vertex},
			{"_ps", D3D::Shader::ShaderType::Pixel},
			{"_gs", D3D::Shader::ShaderType::Geometry},
			{"_ds", D3D::Shader::ShaderType::Domain},
			{"_hs", D3D::Shader::ShaderType::Hull},
			{"_cs", D3D::Shader::ShaderType::Compute},
			{"_ms", D3D::Shader::ShaderType::Mesh},
			{"_as", D3D::Shader::ShaderType::Amplify},
			{"_rh", D3D::Shader::ShaderType::RayHit},
			{"_rm", D3D::Shader::ShaderType::RayMiss},
		};

		D3D::Shader shader{};
		shader.Init(std::string(filename));

		for (int i = 0; i < shaderFiles.size(); i++)
		{
			std::string_view fileExt;
			const auto& lastBackSlash = shaderFiles[i].find_last_of(L'\\');
			const auto& lastForwardSlash = shaderFiles[i].find_last_of(L'/');
			if (lastBackSlash != std::string::npos)
			{
				fileExt = std::string_view(shaderFiles[i].data() + lastBackSlash + 1);
			} 
			else if (lastForwardSlash != std::string::npos)
			{
				fileExt = std::string_view(shaderFiles[i].data() + lastForwardSlash + 1);
			}
			else
			{
				continue;
			}


			const auto& period = fileExt.find_last_of(L'.');
			const auto& underscore = fileExt.find_last_of(L'_');

			//not great, there's still some edge cases but it'll do if we just all keep to conventions :P
			if (underscore == std::string::npos || period == std::string::npos || underscore > period)
			{
				continue;
			}

			std::string_view target = std::string_view(fileExt.data() + underscore, period - underscore);
			std::string_view fileNoExt = std::string_view(fileExt.data(), underscore);

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
			if (m_ColorTargets[i].first.name == filename)
			{
				rtvHandle = (D3D::RTVHandle)i;
				break;
			}
		}

		for (int i = 0; i < m_DepthTargets.size(); i++)
		{
			if (m_DepthTargets[i].first.name == filename)
			{
				dsvHandle = (D3D::DSVHandle)i;
				break;
			}
		}

		for (int i = 0; i < m_SRVs.size(); i++)
		{
			if (m_SRVs[i].first.name == filename)
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

		std::string data = Engine::ReadFileToString(GetFilePath(RENDER_TARGET_FOLDER, filename, ".target"));
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

		bool isDepthTarget = false;
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
				isDepthTarget = true;
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
		else
		{
			Themp::Print("createshaderview was not of type bool or not found!");
		}

		const auto& readFormat = result["format"];
		if (readFormat && readFormat.is_string())
		{
			D3D::TextureFormat formatFromStr{};
			formatFromStr.SetFromString(readFormat.as_string()->get());
			format = D3D::DxTranslator::GetTextureFormat(formatFromStr);
			clearValue.Format = format;
		}
		else
		{
			Themp::Print("format was not of type string or not found!");
		}

		const auto& readWidth = result["fixedwidth"];
		if (readWidth && readWidth.is_integer())
		{
			width = static_cast<int>(readWidth.as_integer()->get());
		}
		else
		{
			Themp::Print("fixedwidth was not of type integer or not found!");
		}

		const auto& readHeight = result["fixedheight"];
		if (readHeight && readHeight.is_integer())
		{
			height = static_cast<int>(readHeight.as_integer()->get());
		}
		else
		{
			Themp::Print("fixedheight was not of type integer or not found!");
		}

		const auto& readCount = result["multisamplecount"];
		if (readCount && readCount.is_integer())
		{
			multisample.Count = static_cast<UINT>(readCount.as_integer()->get());
		}
		else
		{
			Themp::Print("multisamplecount was not of type integer or not found!");
		}

		const auto& readQuality = result["multisamplequality"];
		if (readQuality && readQuality.is_integer())
		{
			multisample.Quality = static_cast<UINT>(readQuality.as_integer()->get());
		}
		else
		{
			Themp::Print("multisamplequality was not of type integer or not found!");
		}

		const auto& resolution = result["resolution"];
		if (resolution && resolution.is_string())
		{
			doesScale = resolution.as_string()->get() != "fixed";
		}
		else
		{
			Themp::Print("resolution was not of type string or not found!");
		}

		const auto& readScale = result["scale"];
		if (readScale && readScale.is_floating_point())
		{
			scale = static_cast<float>(readScale.as_floating_point()->get());
		}
		else
		{
			Themp::Print("scale was not of type float or not found!");
		}

		const auto& clearValues = result["clearvalue"];
		if (clearValues && clearValues.is_array())
		{
			const auto& values = *clearValues.as_array();
			if (isDepthTarget)
			{
				if (values.size() > 0)
				{
					if (values[0].is_floating_point())
					{
						clearValue.DepthStencil.Depth = static_cast<float>(values[0].as_floating_point()->get());
					}
				}
				else
				{
					Themp::Print("clearvalue's first argument needs to be a float (depth) for a depth target!");
				}
				if (values.size() > 1)
				{
					int64_t val = values[1].as_integer()->get();
					if (val > 255 || val < 0)
					{
						Themp::Print("Depth Stencil Clearvalue expected an UINT8, not: %lli", val);
					}
					clearValue.DepthStencil.Stencil = static_cast<UINT8>(val);
				}
				else if(format == DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT)
				{
					Themp::Print("clearvalue's second argument needs to be a integer (stencil) for a depth target!");
				}
			}
			else
			{
				if (values.size() == 4)
				{
					clearValue.Color[0] = static_cast<float>(values[0].as_floating_point()->get());
					clearValue.Color[1] = static_cast<float>(values[1].as_floating_point()->get());
					clearValue.Color[2] = static_cast<float>(values[2].as_floating_point()->get());
					clearValue.Color[3] = static_cast<float>(values[3].as_floating_point()->get());
				}
				else
				{
					Themp::Print("clearvalue needs 4 floats for a color target!");
				}
			}
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
		auto resource = resourceManager.GetTextureResource(device, std::string(filename.begin(),filename.end()), flags, format, 1, multisample, width, height, depth, &clearValue, textureType);
		auto& tex = resourceManager.GetTextureFromResource(device, resource, textureType);

		tex.SetClearValue(clearValue);

		ResourceTextureInfo info;
		info.name = filename;
		info.scalar = scale;
		info.doesScale = doesScale;
		info.clearValue = clearValue;
		switch (textureType)
		{
		case D3D::TEXTURE_TYPE::RTV:
			m_ColorTargets.push_back({ info, tex });
			rtvHandle = m_ColorTargets.size() - 1;
			break;
		case D3D::TEXTURE_TYPE::DSV:
			m_DepthTargets.push_back({ info, tex });
			dsvHandle = m_DepthTargets.size() - 1;
			break;
		}

		if (createSRV)
		{
			auto& srv = resourceManager.GetTextureFromResource(device, resource, D3D::TEXTURE_TYPE::SRV);
			m_SRVs.push_back({ info, srv });
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

	std::vector<D3D::SubPass> Resources::LoadMaterial(const std::string& data, std::vector<std::string>& shaderFiles)
	{
		using namespace Themp::D3D;
		enum MaterialMembers
		{
			POSITIONINFO,
			UVINFO,
			NORMALINFO,
			PASS,
			SHADER,
			COUNT
		};

		const std::string_view validEntries[MaterialMembers::COUNT]
		{
			"positioninfo",
			"uvinfo",
			"normalinfo",
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
				const auto& pos = table[validEntries[POSITIONINFO]];
				const auto& uv = table[validEntries[UVINFO]];
				const auto& normal = table[validEntries[NORMALINFO]];
				if (pass && pass.is_string() && shader && shader.is_string())
				{
					PassHandle passHandle = LoadPass(pass.as_string()->get());
					ShaderHandle shaderHandle = LoadShader(shader.as_string()->get(), shaderFiles);
					bool hasPos = false;
					bool hasUv = false;
					bool hasNormal = false;
					if (pos && pos.is_boolean())
					{
						hasPos = pos.as_boolean()->get();
					}
					if (uv && uv.is_boolean())
					{
						hasUv = uv.as_boolean()->get();
					}
					if (normal && normal.is_boolean())
					{
						hasNormal = normal.as_boolean()->get();
					}
					passData.push_back({ hasPos, hasNormal, hasUv, passHandle , shaderHandle });
				}
				else
				{
					Themp::Print("Pass and/or Shader were not of type string or found");
					Themp::Break();
				}
			}
		}
		return passData;

	}

	void Resources::AddObject3D(D3D::Object3D obj)
	{
		m_3DObjects.push_back(obj);
	}

	const std::vector<Themp::D3D::Object3D>& Resources::GetSceneObjects()
	{
		return m_3DObjects;
	}

	void Resources::LoadScene(std::string sceneFile)
	{
		std::string scenePath = SCENES_FOLDER;
		scenePath.append(sceneFile);

		std::string sceneData = Engine::ReadFileToString(scenePath);
		const toml::parse_result result = toml::parse(Themp::Util::ToLowerCase(sceneData));
		if (!result)
		{
			Themp::Print("%*s at line [%i:%i]", result.error().description().size(), result.error().description().data(), result.error().source().begin.line, result.error().source().begin.column);
			Themp::Break();
		}
		auto GetFloatFromArr = [](const toml::v2::array& arr) 
		{ 
			if (arr.size() == 3 && arr[0].is_floating_point() && arr[1].is_floating_point() && arr[2].is_floating_point())
			{
				return DirectX::XMFLOAT3(static_cast<float>(arr[0].as_floating_point()->get()),
										 static_cast<float>(arr[1].as_floating_point()->get()),
										 static_cast<float>(arr[2].as_floating_point()->get()));
			}
			Themp::Print("float3 value wasn't 3 components or one or more values wasn't a float");
			return DirectX::XMFLOAT3(0, 0, 0);
		};

		for (const auto& rootArrays : result)
		{
			const auto& arr = *rootArrays.second.as_array();
			D3D::Model model = LoadModel(*Engine::instance->m_Renderer, rootArrays.first);
			for(const auto& element : arr)
			{
				const auto& table = *element.as_table();
				D3D::Object3D obj3D{};
				const auto& nameIt = table["name"].as_string();
				if (nameIt)
				{
					obj3D.m_Name = nameIt->get();
				}

				const auto& positionArr = table["position"].as_array();
				DirectX::XMFLOAT3 position{};
				if (positionArr)
				{
					position = GetFloatFromArr(*positionArr);
				}
				const auto& rotationArr = table["rotation"].as_array();
				DirectX::XMFLOAT3 rotation{};
				if (rotationArr)
				{
					rotation = GetFloatFromArr(*rotationArr);
				}
				const auto& scaleArr = table["scale"].as_array();
				DirectX::XMFLOAT3 scale{};
				if (scaleArr)
				{
					scale = GetFloatFromArr(*scaleArr);
				}

				obj3D.m_Transform = Transform(position, rotation, scale);
				
				obj3D.m_Model = Engine::instance->m_Renderer->GetResourceManager().Test_GetAndAddRandomModel();

				obj3D.m_ScriptHandle = Engine::instance->m_Scripting->AddScript("quad");
				Engine::instance->m_Scripting->LinkToObject3D(obj3D.m_ScriptHandle, obj3D.m_Name);

				const auto& materialIt = table["meshmaterials"].as_array();
				if (materialIt)
				{
					const auto& materials = *materialIt;
					for (int i = 0; i < std::min(materials.size(), obj3D.m_Model.m_Meshes.size()); i++)
					{
						const std::string& name = materials[i].as_string()->get();
						for (int j = 0; j < m_Materials.size(); j++)
						{
							if (m_Materials[j].m_Name == name)
							{
								obj3D.m_Model.m_Meshes[i].m_MaterialHandle = j;
								goto CONTINUE;
							}
						}
						Themp::Print("material with name: %s was not found!", name);
					CONTINUE:
						continue;
					}
				}

				m_3DObjects.push_back(obj3D);
				
			}
		}
		
	}


	Themp::D3D::Model Resources::LoadModel(D3D::Control& control, std::string modelName)
	{
		using namespace Themp::D3D;
		for (const auto& model : m_Models)
		{
			if (model.m_Name == modelName)
			{
				return model;
			}
		}

		auto& obj = m_Models.emplace_back();
		const auto& testMesh = Engine::instance->m_Renderer->GetResourceManager().Test_GetAndAddRandomModel();
		obj.m_Meshes = testMesh.m_Meshes;
		obj.m_Name = modelName;
		return obj;
		//for (size_t i = 0; i < modelFiles.size(); i++)
		//{
		//	std::string modelData = ReadFileToString(modelFiles[i]);
		//
		//	const toml::parse_result result = toml::parse(Themp::Util::ToLowerCase(modelData));
		//
		//	std::string name = result["name"].as_string()->get();
		//	std::string material = result["material"].as_string()->get();
		//	auto& obj = m_Models.emplace_back();
		//	const auto& testMesh = Engine::instance->m_Renderer->GetResourceManager().Test_GetAndAddRandomModel();
		//	obj.m_Meshes = testMesh.m_Meshes;
		//	obj.m_Name = GetFileName(modelFiles[i]);
		//
		//	const auto& iterator = std::find_if(m_Materials.begin(), m_Materials.end(), [&](D3D::Material& it) { return it.m_Name == material; } );
		//	if (iterator != m_Materials.end())
		//	{
		//		control.AddMeshToDraw(obj, *this, iterator->m_SubPasses);
		//	}
		//	return obj;
		//}
	}
}