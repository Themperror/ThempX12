#include "subpass.h"


using namespace Themp::D3D;

SubPass::SubPass(ShaderHandle shaderID)
{
	m_ShaderID = shaderID;
}

void SubPass::Register(const MeshData& mesh)
{
	m_RenderList.push_back(mesh);
}