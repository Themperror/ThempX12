#pragma once

#include <vector>
#include "renderer/types.h"
#include "subpass.h"

#include <string>

namespace Themp::D3D
{
	class MainPass
	{
	public:
		SubPassHandle AddSubPass(ShaderHandle material);
		SubPassHandle AddSubPass(const SubPass& subpass);
		
	private:
		std::vector<SubPass> m_SubPasses;
		std::string name;
	protected:
		explicit MainPass(const std::string& name);

		friend class Control;
	};
	
}