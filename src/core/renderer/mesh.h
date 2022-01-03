#pragma once

#include "types.h"

namespace Themp::D3D
{
	class Mesh
	{
	public:
		MaterialHandle m_MaterialHandle;
		MeshData m_MeshData;
		std::vector<DirectX::XMFLOAT4X4> m_Transform;
	};
}