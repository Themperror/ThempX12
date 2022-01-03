#pragma once

#include "core/renderer/types.h"
#include "core/components/transform.h"
#include "core/renderer/model.h"
namespace Themp
{
	class SceneObject
	{
	public:
		//std::vector<Mesh> m_Meshes;
		Transform m_Transform = Transform(DirectX::XMFLOAT3(0,0,0), DirectX::XMFLOAT3(0,0,0), DirectX::XMFLOAT3(1,1,1));
		D3D::ModelHandle m_ModelHandle;
		bool m_Visible = true;
		size_t m_ID;
		std::string m_Name;
		std::vector<D3D::MaterialHandle> m_OverrideMaterials;
		Scripting::ScriptHandle m_ScriptHandle;
	};
}