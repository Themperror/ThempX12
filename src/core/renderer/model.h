#pragma once

#include "types.h"
namespace Themp::D3D
{
	class Model
	{
	public:
		std::string m_Name;
		std::vector<D3D::MeshHandle> m_Meshes;
	};
}