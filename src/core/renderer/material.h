#pragma once

#include "types.h"

namespace Themp::D3D
{
	class Material
	{
	public:
		std::string m_Name;
		std::vector<SubPassHandle> m_SubPasses;
		std::vector<TextureHandle> m_Textures;
	};
}