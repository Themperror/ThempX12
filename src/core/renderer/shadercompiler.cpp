#include "core/renderer/shadercompiler.h"
#include "core/engine.h"
#include "core/util/print.h"
#include "core/util/break.h"
#include "core/util/svars.h"
#include "core/util/stringUtils.h"

#include <unordered_map>
#include <string>
#include <vector>

#define SHADER_DATA_FOLDER L"../data/shaders/"
#define SHADER_RESOURCES_FOLDER L"../resources/shaders/"
namespace Themp::D3D
{
	void ShaderCompiler::Init()
	{
		// 
		// Create compiler and utils.
		//

		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));
		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));

		//
		// Create default include handler. (You can create your own...)
		//
		pUtils->CreateDefaultIncludeHandler(&pIncludeHandler);
	}

	void ShaderCompiler::CompileIndividual(D3D::Shader::SourcePair& src) const
	{
		bool debug = Themp::Engine::instance->s_SVars.GetSVarInt(Themp::SVar::iDebugShaders);

		static const std::unordered_map<D3D::Shader::ShaderType, std::wstring_view> TypeToTarget =
		{
			{ D3D::Shader::ShaderType::Vertex	,L"vs"	},
			{ D3D::Shader::ShaderType::Pixel	,L"ps"	},
			{ D3D::Shader::ShaderType::Geometry	,L"gs"	},
			{ D3D::Shader::ShaderType::Domain	,L"ds"	},
			{ D3D::Shader::ShaderType::Hull		,L"hs"	},
			{ D3D::Shader::ShaderType::Compute	,L"cs"	},
			//these are "guesses", and not shown in DXC yet
			{ D3D::Shader::ShaderType::Mesh		,L"ms"	},
			{ D3D::Shader::ShaderType::Amplify	,L"as"	},
			{ D3D::Shader::ShaderType::RayHit	,L"rh"	},
			{ D3D::Shader::ShaderType::RayMiss	,L"rm"	},
		};

		std::wstring_view target = TypeToTarget.find(src.type)->second;

		std::vector<std::wstring> compileArgs;
		std::wstring wideName = Themp::Util::ToWideString(src.name);
		compileArgs.reserve(16);

		compileArgs.emplace_back(wideName);
		compileArgs.emplace_back(L"-E");
		compileArgs.emplace_back(L"main");

		compileArgs.emplace_back(L"-I");
		compileArgs.emplace_back(SHADER_RESOURCES_FOLDER);

		compileArgs.emplace_back(L"-T");
		compileArgs.emplace_back(target).append(L"_6_0");

		compileArgs.emplace_back(L"-Fo");
		compileArgs.emplace_back(SHADER_DATA_FOLDER).append(wideName).append(L".").append(target);

		if (debug)
		{
			compileArgs.emplace_back(L"-Zs");
			compileArgs.emplace_back(L"-Fd");
			compileArgs.emplace_back(SHADER_DATA_FOLDER).append(wideName).append(L".").append(target).append(L".pdb");
		}


		std::vector<LPCWSTR> args;
		args.resize(compileArgs.size());
		for (int i = 0; i < args.size(); i++)
		{
			args[i] = compileArgs[i].c_str();
		}

		//
		// Open source file.  
		//
		ComPtr<IDxcBlobEncoding> pSource = nullptr;
		std::wstring fileName = SHADER_RESOURCES_FOLDER;
		fileName.append(wideName);
		fileName.append(L"_").append(target);
		fileName.append(L".hlsl");
		pUtils->LoadFile(fileName.c_str(), nullptr, &pSource);
		DxcBuffer Source;
		Source.Ptr = pSource->GetBufferPointer();
		Source.Size = pSource->GetBufferSize();
		Source.Encoding = DXC_CP_ACP; // Assume BOM says UTF8 or UTF16 or this is ANSI text.


		//
		// Compile it with specified arguments.
		//
		ComPtr<IDxcResult> pResults;
		pCompiler->Compile(
			&Source,                // Source buffer.
			args.data(),                // Array of pointers to arguments.
			static_cast<UINT32>(args.size()),      // Number of arguments.
			pIncludeHandler.Get(),        // User-provided interface to handle #include directives (optional).
			IID_PPV_ARGS(&pResults) // Compiler output status, buffer, and errors.
		);

		//
		// Print errors if present.
		//
		ComPtr<IDxcBlobUtf8> pErrors = nullptr;
		pResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
		// Note that d3dcompiler would return null if no errors or warnings are present.  
		// IDxcCompiler3::Compile will always return an error buffer, but its length will be zero if there are no warnings or errors.
		if (pErrors != nullptr && pErrors->GetStringLength() != 0)
		{
			Themp::Print("Warnings and Errors:\n%s\n", pErrors->GetStringPointer());
			Themp::Break();
		}
		//
		// Quit if the compilation failed.
		//
		HRESULT hrStatus;
		pResults->GetStatus(&hrStatus);
		if (FAILED(hrStatus))
		{
			Themp::Print("Compilation Failed");
			Themp::Break();
		}

		//
		// Save shader binary.
		//
		ComPtr<IDxcBlob> pShader = nullptr;
		ComPtr<IDxcBlobUtf16> pShaderName = nullptr;
		pResults->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pShader), &pShaderName);
		if (pShader != nullptr && debug)
		{
			FILE* fp = NULL;
			CreateDirectoryW(L"../data", NULL);
			CreateDirectoryW(L"../data/shaders/", NULL);
			_wfopen_s(&fp, pShaderName->GetStringPointer(), L"wb");
			fwrite(pShader->GetBufferPointer(), pShader->GetBufferSize(), 1, fp);
			fclose(fp);
		}
		src.data = pShader;

		//
		// Save pdb.
		//
		ComPtr<IDxcBlob> pPDB = nullptr;
		ComPtr<IDxcBlobUtf16> pPDBName = nullptr;
		pResults->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pPDB), &pPDBName);
		if(pPDB != nullptr && debug)
		{
			FILE* fp = NULL;

			// Note that if you don't specify -Fd, a pdb name will be automatically generated. Use this file name to save the pdb so that PIX can find it quickly.
			_wfopen_s(&fp, pPDBName->GetStringPointer(), L"wb");
			fwrite(pPDB->GetBufferPointer(), pPDB->GetBufferSize(), 1, fp);
			fclose(fp);
		}

		//
		// Print hash.
		//
		//ComPtr<IDxcBlob> pHash = nullptr;
		//pResults->GetOutput(DXC_OUT_SHADER_HASH, IID_PPV_ARGS(&pHash), nullptr);
		//if (pHash != nullptr)
		//{
		//	wprintf(L"Hash: ");
		//	DxcShaderHash* pHashBuf = (DxcShaderHash*)pHash->GetBufferPointer();
		//	for (int i = 0; i < _countof(pHashBuf->HashDigest); i++)
		//		wprintf(L"%x", pHashBuf->HashDigest[i]);
		//	wprintf(L"\n");
		//}


		//
		// Get separate reflection.
		//
		//ComPtr<IDxcBlob> pReflectionData;
		//pResults->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&pReflectionData), nullptr);
		//if (pReflectionData != nullptr)
		//{
		//	// Optionally, save reflection blob for later here.
		//
		//	// Create reflection interface.
		//	DxcBuffer ReflectionData;
		//	ReflectionData.Encoding = DXC_CP_ACP;
		//	ReflectionData.Ptr = pReflectionData->GetBufferPointer();
		//	ReflectionData.Size = pReflectionData->GetBufferSize();
		//
		//	ComPtr< ID3D12ShaderReflection > pReflection;
		//	pUtils->CreateReflection(&ReflectionData, IID_PPV_ARGS(&pReflection));
		//
		//	// Use reflection interface here.
		//
		//}
	}
	void ShaderCompiler::Compile(Shader& shader) const
	{
		for (int i = 0; i < shader.m_ShaderPairs.size(); i++)
		{
			CompileIndividual(shader.m_ShaderPairs[i]);
		}
	}
}