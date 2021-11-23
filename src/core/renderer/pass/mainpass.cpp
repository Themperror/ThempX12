#include "mainpass.h"

using namespace Themp::D3D;

MainPass::MainPass(const std::string& name)
{

}

SubPassHandle MainPass::AddSubPass(MaterialHandle materialID)
{
	m_SubPasses.push_back(SubPass{ materialID });
	return static_cast<SubPassHandle>(m_SubPasses.size() - 1);
}
SubPassHandle MainPass::AddSubPass(const SubPass& subpass)
{
	m_SubPasses.push_back(subpass);
	return static_cast<SubPassHandle>(m_SubPasses.size() - 1);
}