#pragma once

#include <vector>
#include <string>
#include "types.h"
#include <lib/dxcapi.h>
namespace Themp::D3D
{
	class Shader
	{
	public:
		enum ShaderType
		{
			Vertex,
			Pixel,
			Geometry,
			Hull,
			Domain,
			Compute,
			Mesh,
			Amplify,
			RayHit,
			RayMiss
		};
		struct SourcePair
		{
			std::wstring name;
			ShaderType type;
			ComPtr<IDxcBlob> data;
		};
		void Init(const std::string& name);
		void AddShaderSource(std::wstring name, ShaderType type);
		bool IsValid() const;
		std::string_view GetName() const { return m_Name; }
		const std::vector<SourcePair>& GetShaders() const;
	private:
		std::string m_Name;
		std::vector<SourcePair> m_ShaderPairs;

		friend class ShaderCompiler;
		friend class Resources;
	};
}