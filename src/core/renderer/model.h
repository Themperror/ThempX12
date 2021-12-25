#pragma once

#include "core/renderer/mesh.h"
namespace Themp::D3D
{
	class Model
	{
	public:
		std::string m_Name;
		std::vector<Mesh> m_Meshes;
	};
}