#pragma once

#include <vector>
#include <string>
#include "types.h"
namespace Themp::D3D
{

	class Shader
	{
	public:
		bool AddShaderSource(std::wstring path);
		void Recompile();

	private:
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
			ShaderType type;
			std::wstring path;
		};
		std::vector<SourcePair> m_ShaderPairs;
	};
}