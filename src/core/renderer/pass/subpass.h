#pragma once

#include <vector>
#include "renderer/types.h"

namespace Themp::D3D
{
	class SubPass
	{
	public:
		explicit SubPass(ShaderHandle materialID);
		void Register(const MeshData& mesh);

	private:
		std::vector<MeshData> m_RenderList;
		ShaderHandle m_ShaderID;
	};
}