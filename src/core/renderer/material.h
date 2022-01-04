#pragma once

#include "types.h"

namespace Themp::D3D
{
	class Material
	{
	public:
		struct SubPassData
		{
			SubPassHandle handle;
			std::vector<TextureTypePair> textures;
		};
		std::string m_Name;
		std::vector<SubPassData> m_SubPasses;
	};
}