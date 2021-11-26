#include "pass.h"
#include "util/print.h"
#include "util/break.h"
using namespace Themp::D3D;

Pass::Pass(std::string_view name)
{
	m_Name = name;
	m_Priority = std::numeric_limits<int>::lowest();
	m_SampleMask = 0;
	m_MultisampleCount = 1;
	m_MultisampleQuality = 0;
	m_RasterState= {};
	m_DepthState = {};
	for (int i = 0; i < m_BlendStates.size(); i++)
	{
		m_BlendStates[i] = {};
	}
	for (int i = 0; i < m_RenderTargets.size(); i++)
	{
		m_RenderTargets[i] = InvalidHandle;
	}
	m_DepthTarget = InvalidHandle;
}

void Pass::SetName(std::string_view name)
{
	m_Name = name;
}
std::string_view Pass::GetName() const
{
	return m_Name;
}

void Pass::SetPriority(int priority)
{
	m_Priority = priority;
}
void Pass::SetDepthEnable(bool enabled)
{
	m_DepthState.depthEnable = enabled;
}
void Pass::SetDepthWriteMask(std::string_view str)
{ 
	for (int i = 0; i < DepthWriteMask::COUNT; i++)
	{
		if (str == DepthWriteMask::Str[i])
		{
			m_DepthState.depthWriteMask.mask = (DepthWriteMask::Mask)i;
			break;
		}
	}
}
void Pass::SetDepthFunc(std::string_view str)
{
	for (int i = 0; i < ComparisonFunc::COUNT; i++)
	{
		if (str == ComparisonFunc::Str[i])
		{
			m_DepthState.depthFunc.func = (ComparisonFunc::Func)i;
			break;
		}
	}
}
void Pass::SetStencilEnable(bool enabled)
{
	m_DepthState.stencilEnable = enabled;
}
void Pass::SetStencilReadMask(uint8_t mask)
{
	m_DepthState.stencilReadMask = mask;
}
void Pass::SetStencilWriteMask(uint8_t mask)
{
	m_DepthState.stencilWriteMask = mask;
}
void Pass::SetFrontFaceFailOp(std::string_view str)
{
	for (int i = 0; i < StencilOp::COUNT; i++)
	{
		if (str == StencilOp::Str[i])
		{
			m_DepthState.frontFace.stencilFailOp.op = (StencilOp::Op)i;
			break;
		}
	}
}
void Pass::SetFrontFaceDepthFailOp(std::string_view str)
{
	for (int i = 0; i < StencilOp::COUNT; i++)
	{
		if (str == StencilOp::Str[i])
		{
			m_DepthState.frontFace.stencilDepthFailOp.op = (StencilOp::Op)i;
			break;
		}
	}
}
void Pass::SetFrontFacePassOp(std::string_view str)
{
	for (int i = 0; i < StencilOp::COUNT; i++)
	{
		if (str == StencilOp::Str[i])
		{
			m_DepthState.frontFace.stencilPassOp.op = (StencilOp::Op)i;
			break;
		}
	}
}
void Pass::SetFrontFaceFunc(std::string_view str)
{
	for (int i = 0; i < ComparisonFunc::COUNT; i++)
	{
		if (str == ComparisonFunc::Str[i])
		{
			m_DepthState.frontFace.stencilFunc.func = (ComparisonFunc::Func)i;
			break;
		}
	}
}
void Pass::SetBackFaceFailOp(std::string_view str)
{
	for (int i = 0; i < StencilOp::COUNT; i++)
	{
		if (str == StencilOp::Str[i])
		{
			m_DepthState.backFace.stencilFailOp.op = (StencilOp::Op)i;
			break;
		}
	}
}
void Pass::SetBackFaceDepthFailOp(std::string_view str)
{
	for (int i = 0; i < StencilOp::COUNT; i++)
	{
		if (str == StencilOp::Str[i])
		{
			m_DepthState.backFace.stencilDepthFailOp.op = (StencilOp::Op)i;
			break;
		}
	}
}
void Pass::SetBackFacePassOp(std::string_view str)
{
	for (int i = 0; i < StencilOp::COUNT; i++)
	{
		if (str == StencilOp::Str[i])
		{
			m_DepthState.backFace.stencilPassOp.op = (StencilOp::Op)i;
			break;
		}
	}
}
void Pass::SetBackFaceFunc(std::string_view str)
{
	for (int i = 0; i < ComparisonFunc::COUNT; i++)
	{
		if (str == ComparisonFunc::Str[i])
		{
			m_DepthState.backFace.stencilFunc.func = (ComparisonFunc::Func)i;
			break;
		}
	}
}
void Pass::SetFrontCounterClockwise(bool val)
{
	m_RasterState.frontCounterClockwise = val;
}
void Pass::SetFillMode(std::string_view str)
{
	for (int i = 0; i < FillMode::COUNT; i++)
	{
		if (str == FillMode::Str[i])
		{
			m_RasterState.fillMode.mode = (FillMode::Mode)i;
			break;
		}
	}
}
void Pass::SetCullMode(std::string_view str)
{
	for (int i = 0; i < CullMode::COUNT; i++)
	{
		if (str == CullMode::Str[i])
		{
			m_RasterState.cullMode.mode = (CullMode::Mode)i;
			break;
		}
	}
}
void Pass::SetDepthBias(int bias)
{
	m_RasterState.depthBias = bias;
}
void Pass::SetDepthBiasClamp(float clamp)
{
	m_RasterState.depthBiasClamp = clamp;
}
void Pass::SetSlopeScaledDepthBias(float bias)
{
	m_RasterState.slopeScaledDepthBias = bias;
}
void Pass::SetDepthClipEnable(bool enabled)
{
	m_RasterState.depthClipEnable = enabled;
}
void Pass::SetMultisampleEnable(bool enabled)
{
	m_RasterState.multisampleEnable = enabled;
}
void Pass::SetAntialiasedLineEnable(bool enabled)
{
	m_RasterState.antialiasedLineEnable = enabled;
}
void Pass::SetForcedSampleCount(unsigned int count)
{
	m_RasterState.forcedSampleCount = count;
}
void Pass::SetAlphaToCoverageEnable(bool enabled)
{
	m_RasterState.alphaToCoverageEnable = enabled;
}
void Pass::SetIndependendBlendEnable(bool enabled)
{
	m_RasterState.independendBlendEnable = enabled;
}
void Pass::SetSampleMask(uint32_t mask)
{
	m_SampleMask = mask;
}
void Pass::SetMultisampleCount(int count)
{
	m_MultisampleCount = count;
}
void Pass::SetMultisampleQuality(int quality)
{
	m_MultisampleQuality = quality;
}
void Pass::SetDepthTarget(RenderTargetHandle handle)
{
	m_DepthTarget = handle;
}
void Pass::SetColorTarget(int index, RenderTargetHandle handle)
{
	if (index < 0 || index >= 8)
	{
		Themp::Print("Slot %i is out of range for rendertarget [0,7]!", index);
		Themp::Break();
		return;
	}
	m_RenderTargets[index] = handle;
}
void Pass::SetBlendTarget(int index, const BlendState& state)
{
	if (index < 0 || index >= 8)
	{
		Themp::Print("Slot %i is out of range for blendstate [0,7]!", index);
		Themp::Break();
		return;
	}
	m_BlendStates[index] = state;
}

bool Pass::IsValid() const 
{ 
	bool invalid = false;
	invalid |= m_Name.size() == 0;
	invalid |= m_Priority == std::numeric_limits<int>::lowest();
	return !invalid;
}