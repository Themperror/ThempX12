#pragma once

#include <dxgi1_6.h>
#include <wrl.h>
#include <src/lib/d3d12shader.h>
#include <src/lib/dxcapi.h>
#include "src/core/renderer/shader.h"

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