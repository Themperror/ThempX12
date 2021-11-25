#include "pass.h"

using namespace Themp::D3D;

Pass::Pass(std::string_view name)
{
	m_Name = name;
	m_Priority = std::numeric_limits<int>::lowest();
}

void Pass::AddRenderable(ShaderHandle materialID)
{

}

void Pass::AddRenderable(ShaderHandle material, const MeshData& mesh)
{

}

void Pass::Draw(Context& context)
{

}


void Pass::SetName(std::string_view name)
{
	m_Name = name;
}

void Pass::SetPriority(int priority)
{
	m_Priority = priority;
}