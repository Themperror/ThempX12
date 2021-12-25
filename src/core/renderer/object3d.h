#pragma once

#include "core/renderer/types.h"
#include "core/components/transform.h"
#include "core/renderer/model.h"
namespace Themp::D3D
{
	class Object3D
	{
	public:
		//std::vector<Mesh> m_Meshes;
		Transform m_Transform = Transform(DirectX::XMFLOAT3(0,0,0), DirectX::XMFLOAT3(0,0,0), DirectX::XMFLOAT3(1,1,1));
		Model m_Model;
		bool m_Visible = true;
		size_t m_ID;
		std::string m_Name;
		ScriptHandle m_ScriptHandle;
	};
}