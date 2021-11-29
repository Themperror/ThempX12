#pragma once

#include <dxgi1_6.h>
#include <wrl.h>
#include <lib/d3d12shader.h>
#include <lib/dxcapi.h>
#include "shader.h"

using namespace Microsoft::WRL;
namespace Themp::D3D
{
	class ShaderCompiler
	{
	public:
		void Init();
		void Compile(Shader& shader) const;
	private:
		void CompileIndividual(Shader::SourcePair& src) const;

		ComPtr<IDxcUtils> pUtils;
		ComPtr<IDxcCompiler3> pCompiler;
		ComPtr<IDxcIncludeHandler> pIncludeHandler;
	};
}