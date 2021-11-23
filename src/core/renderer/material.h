#pragma once

#include <vector>
#include "renderer/pass/mainpass.h"

#include <string>

namespace Themp::D3D
{
	class Material
	{
	public:
		bool ParseMaterialFile(const std::string& data);
	private:
		std::vector<MainPass> m_MainPasses;


	};

}