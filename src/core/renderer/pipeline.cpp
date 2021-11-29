#include "renderer/pipeline.h"
#include "renderer/control.h"
#include "renderer/device.h"
#include "engine.h"
#include "resources.h"
#include "util/print.h"
#include "util/break.h"
namespace Themp
{
	namespace D3D
	{
		void SetBlendState(D3D12_RENDER_TARGET_BLEND_DESC& desc, const Themp::D3D::Pass::BlendState& src)
		{
			desc.BlendEnable = src.blendEnable;
			desc.BlendOp = DxTranslator::GetBlendOp(src.blendOp);
			desc.BlendOpAlpha = DxTranslator::GetBlendOp(src.blendAlphaOp);
			desc.DestBlend = DxTranslator::GetBlend(src.destBlend);
			desc.DestBlendAlpha = DxTranslator::GetBlend(src.destBlendAlpha);
			desc.LogicOp = DxTranslator::GetLogicOp(src.logicOp);
			desc.LogicOpEnable = src.logicOpEnable;
			desc.RenderTargetWriteMask = src.renderTargetWriteMask;
			desc.SrcBlend = DxTranslator::GetBlend(src.srcBlend);
			desc.SrcBlendAlpha = DxTranslator::GetBlend(src.srcBlendAlpha);
		}

		void Pipeline::Init(const SubPass& subpass)
		{
			const Pass& pass = Themp::Engine::instance->m_Resources->Get(subpass.pass);
			const Shader& shader = Themp::Engine::instance->m_Resources->Get(subpass.shader);

			D3D12_GRAPHICS_PIPELINE_STATE_DESC desc {};
			
			desc.BlendState.AlphaToCoverageEnable = pass.m_RasterState.alphaToCoverageEnable;
			desc.BlendState.IndependentBlendEnable = pass.m_RasterState.independendBlendEnable;
			for (int i = 0; i < pass.m_BlendStates.size(); i++)
			{
				SetBlendState(desc.BlendState.RenderTarget[i], pass.m_BlendStates[i]);
			}

			desc.DepthStencilState.BackFace.StencilDepthFailOp = DxTranslator::GetStencilOp(pass.m_DepthState.backFace.stencilDepthFailOp);
			desc.DepthStencilState.BackFace.StencilFailOp = DxTranslator::GetStencilOp(pass.m_DepthState.backFace.stencilFailOp);
			desc.DepthStencilState.BackFace.StencilFunc = DxTranslator::GetComparisonFunc(pass.m_DepthState.backFace.stencilFunc);
			desc.DepthStencilState.BackFace.StencilPassOp = DxTranslator::GetStencilOp(pass.m_DepthState.backFace.stencilPassOp);
			desc.DepthStencilState.FrontFace.StencilDepthFailOp = DxTranslator::GetStencilOp(pass.m_DepthState.frontFace.stencilDepthFailOp);
			desc.DepthStencilState.FrontFace.StencilFailOp = DxTranslator::GetStencilOp(pass.m_DepthState.frontFace.stencilFailOp);
			desc.DepthStencilState.FrontFace.StencilFunc = DxTranslator::GetComparisonFunc(pass.m_DepthState.frontFace.stencilFunc);
			desc.DepthStencilState.FrontFace.StencilPassOp = DxTranslator::GetStencilOp(pass.m_DepthState.frontFace.stencilPassOp);
			desc.DepthStencilState.DepthEnable = pass.m_DepthState.depthEnable;
			desc.DepthStencilState.DepthFunc = DxTranslator::GetComparisonFunc(pass.m_DepthState.depthFunc);
			desc.DepthStencilState.DepthWriteMask = DxTranslator::GetDepthWriteMask(pass.m_DepthState.depthWriteMask);
			desc.DepthStencilState.StencilEnable = pass.m_DepthState.stencilEnable;
			desc.DepthStencilState.StencilReadMask = pass.m_DepthState.stencilReadMask;
			desc.DepthStencilState.StencilWriteMask = pass.m_DepthState.stencilWriteMask;
			
			desc.RasterizerState.AntialiasedLineEnable = pass.m_RasterState.antialiasedLineEnable;
			desc.RasterizerState.CullMode = DxTranslator::GetCullMode(pass.m_RasterState.cullMode);
			desc.RasterizerState.DepthBias = pass.m_RasterState.depthBias;
			desc.RasterizerState.DepthBiasClamp = pass.m_RasterState.depthBiasClamp;
			desc.RasterizerState.DepthClipEnable = pass.m_RasterState.depthClipEnable;
			desc.RasterizerState.FillMode = DxTranslator::GetFillMode(pass.m_RasterState.fillMode);
			desc.RasterizerState.ForcedSampleCount = pass.m_RasterState.forcedSampleCount;
			desc.RasterizerState.FrontCounterClockwise = pass.m_RasterState.frontCounterClockwise;
			desc.RasterizerState.MultisampleEnable = pass.m_RasterState.multisampleEnable;
			desc.RasterizerState.SlopeScaledDepthBias = pass.m_RasterState.slopeScaledDepthBias;

			desc.RasterizerState.ConservativeRaster = DxTranslator::GetConservativeRaster(pass.m_RasterState.conservativeRaster);

			desc.SampleDesc.Count = pass.m_MultisampleCount;
			desc.SampleDesc.Quality = pass.m_MultisampleQuality;
			desc.SampleMask = pass.m_SampleMask;

			desc.PrimitiveTopologyType = DxTranslator::GetTopology(pass.m_Topology);

			int numValidTargets = 0;
			for (int i = 0; i < pass.m_RenderTargets.size(); i++)
			{
				desc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
				if (pass.m_RenderTargets[i].rtv.IsValid())
				{
					Texture& tex = Themp::Engine::instance->m_Resources->Get(pass.m_RenderTargets[i].rtv);
					desc.RTVFormats[numValidTargets] = tex.GetResource(D3D::TEXTURE_TYPE::RTV)->GetDesc().Format;
					numValidTargets++;
				}
			}
			desc.NumRenderTargets = numValidTargets;

			desc.DSVFormat = DXGI_FORMAT_UNKNOWN;
			if (pass.m_DepthTarget.dsv.IsValid())
			{
				Texture& tex = Themp::Engine::instance->m_Resources->Get(pass.m_DepthTarget.dsv);
				desc.DSVFormat = tex.GetResource(D3D::TEXTURE_TYPE::DSV)->GetDesc().Format;
			}


			const auto& shaders = shader.GetShaders();
			for (int i = 0; i < shaders.size(); i++)
			{
				const auto& data = shaders[i].data;
				if (data != nullptr)
				{
					switch (shaders[i].type)
					{
					case Shader::ShaderType::Vertex:
						desc.VS.BytecodeLength = data->GetBufferSize();
						desc.VS.pShaderBytecode = data->GetBufferPointer();
						break;
					case Shader::ShaderType::Pixel:
						desc.PS.BytecodeLength = data->GetBufferSize();
						desc.PS.pShaderBytecode = data->GetBufferPointer();
						break;
					case Shader::ShaderType::Geometry:
						desc.GS.BytecodeLength = data->GetBufferSize();
						desc.GS.pShaderBytecode = data->GetBufferPointer();
						break;
					case Shader::ShaderType::Hull:
						desc.HS.BytecodeLength = data->GetBufferSize();
						desc.HS.pShaderBytecode = data->GetBufferPointer();
						break;
					case Shader::ShaderType::Domain:
						desc.DS.BytecodeLength = data->GetBufferSize();
						desc.DS.pShaderBytecode = data->GetBufferPointer();
						break;
					}
				}
			}


			if (Themp::Engine::instance->m_Renderer->GetDevice()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&m_Pipeline)) != S_OK)
			{
				Themp::Print("Failed to create pipeline state!");
				Themp::Break();
			}
		}
	}
}