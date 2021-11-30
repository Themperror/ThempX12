#include "renderer/pipeline.h"
#include "renderer/control.h"
#include "renderer/device.h"
#include "engine.h"

#include "util/svars.h"
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

			m_PassHandle = subpass.pass;
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
					m_RenderTargets.push_back(tex.GetCPUHandle());
				}
			}
			desc.NumRenderTargets = numValidTargets;

			desc.DSVFormat = DXGI_FORMAT_UNKNOWN;
			if (pass.m_DepthTarget.dsv.IsValid())
			{
				Texture& tex = Themp::Engine::instance->m_Resources->Get(pass.m_DepthTarget.dsv);
				desc.DSVFormat = tex.GetResource(D3D::TEXTURE_TYPE::DSV)->GetDesc().Format;
				m_DepthTarget = tex.GetCPUHandle();
			}
			else
			{
				m_DepthTarget = {};
			}

			int height = Engine::s_SVars.GetSVarInt(SVar::iWindowHeight);
			int width = Engine::s_SVars.GetSVarInt(SVar::iWindowWidth);


			for (int i = 0; i < m_Viewports.size(); i++)
			{
				m_Viewports[i].Height = pass.m_Viewports[i].height;
				m_Viewports[i].Width = pass.m_Viewports[i].width;
				m_Viewports[i].MaxDepth = pass.m_Viewports[i].maxDepth;
				m_Viewports[i].MinDepth = pass.m_Viewports[i].minDepth;
				m_Viewports[i].TopLeftX = pass.m_Viewports[i].topLeftX;
				m_Viewports[i].TopLeftY = pass.m_Viewports[i].topLeftY;

				if (!pass.m_Viewports[i].fixedResolution)
				{
					m_Viewports[i].Height = height * pass.m_Viewports[i].scaler;
					m_Viewports[i].Width = width * pass.m_Viewports[i].scaler;
				}
			}
			for (int i = 0; i < m_Scissors.size(); i++)
			{
				m_Scissors[i].bottom = pass.m_Scissors[i].bottom;
				m_Scissors[i].right = pass.m_Scissors[i].right;
				m_Scissors[i].left = pass.m_Scissors[i].left;
				m_Scissors[i].top = pass.m_Scissors[i].top;

				if (!pass.m_Scissors[i].fixedResolution)
				{
					m_Scissors[i].bottom = height * pass.m_Scissors[i].scaler;
					m_Scissors[i].right = width * pass.m_Scissors[i].scaler;
				}
			}

			std::vector<D3D12_INPUT_ELEMENT_DESC> iaLayouts;
			int byteOffSet = 0;
			int currentSlot = 0;
			if (subpass.NeedsPositionInfo)
			{
				D3D12_INPUT_ELEMENT_DESC& iaDesc = iaLayouts.emplace_back();
				iaDesc.AlignedByteOffset = 0;
				iaDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;
				iaDesc.InputSlot = 0;
				iaDesc.InputSlotClass = D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				iaDesc.SemanticName = "POSITION";
			}
			if (subpass.NeedsNormalInfo)
			{
				D3D12_INPUT_ELEMENT_DESC& iaDesc = iaLayouts.emplace_back();
				iaDesc.AlignedByteOffset = 12;
				iaDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;
				iaDesc.InputSlot = 1;
				iaDesc.InputSlotClass = D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				iaDesc.SemanticName = "NORMAL";
			}
			if (subpass.NeedsUVInfo)
			{
				D3D12_INPUT_ELEMENT_DESC& iaDesc = iaLayouts.emplace_back();
				iaDesc.AlignedByteOffset = 24;
				iaDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT;
				iaDesc.InputSlot = 2;
				iaDesc.InputSlotClass = D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				iaDesc.SemanticName = "UV";
			}

			desc.InputLayout.NumElements = iaLayouts.size();
			desc.InputLayout.pInputElementDescs = iaLayouts.data();

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
			if (shaders.size() > 0)
			{
				ComPtr<ID3D12RootSignatureDeserializer> rootSignatureDeserializer;
				D3D12CreateRootSignatureDeserializer(shaders[0].data->GetBufferPointer(), shaders[0].data->GetBufferSize(), IID_PPV_ARGS(&rootSignatureDeserializer));
				if (rootSignatureDeserializer)
				{
					auto rootDesc = rootSignatureDeserializer->GetRootSignatureDesc();
					ComPtr<ID3DBlob> signature;
					ComPtr<ID3DBlob> error;
					D3D12SerializeRootSignature(rootDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, &error);
					
					Themp::Engine::instance->m_Renderer->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
					desc.pRootSignature = m_RootSignature.Get();
				}
			}

			if (Themp::Engine::instance->m_Renderer->GetDevice()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&m_Pipeline)) != S_OK)
			{
				Themp::Print("Failed to create pipeline state!");
				Themp::Break();
			}
			m_Pipeline->SetName(std::wstring(pass.m_Name.begin(), pass.m_Name.end()).c_str());
		}


		void Pipeline::SetTo(ComPtr<ID3D12GraphicsCommandList> cmdList)
		{
			const Pass& pass = Themp::Engine::instance->m_Resources->Get(m_PassHandle);

			for (int i = 0; i < m_RenderTargets.size(); i++)
			{
				const auto& tex = Themp::Engine::instance->m_Resources->Get(pass.m_RenderTargets[i].rtv);
				cmdList->ClearRenderTargetView(m_RenderTargets[i], tex.GetClearValue().Color, 0, nullptr);
			}

			if (pass.m_DepthTarget.dsv.IsValid())
			{
				const auto& tex = Themp::Engine::instance->m_Resources->Get(pass.m_DepthTarget.dsv);
				D3D12_CLEAR_FLAGS flags = D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH;
				if (tex.GetResource(TEXTURE_TYPE::DSV)->GetDesc().Format == DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT)
				{
					flags |= D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_STENCIL;
				}
				cmdList->ClearDepthStencilView(m_DepthTarget, flags, tex.GetClearValue().DepthStencil.Depth, tex.GetClearValue().DepthStencil.Stencil, 0, nullptr);
			}
			

			cmdList->SetPipelineState(m_Pipeline.Get());
			cmdList->SetGraphicsRootSignature(m_RootSignature.Get());
			cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			cmdList->OMSetRenderTargets(m_RenderTargets.size(), m_RenderTargets.data(), false, m_DepthTarget.ptr != 0 ? &m_DepthTarget : nullptr);

			int height = Engine::s_SVars.GetSVarInt(SVar::iWindowHeight);
			int width = Engine::s_SVars.GetSVarInt(SVar::iWindowWidth);


			for (int i = 0; i < m_Viewports.size(); i++)
			{
				if (!pass.m_Viewports[i].fixedResolution)
				{
					m_Viewports[i].Height = height * pass.m_Viewports[i].scaler;
					m_Viewports[i].Width = width * pass.m_Viewports[i].scaler;
				}
			}
			for (int i = 0; i < m_Scissors.size(); i++)
			{
				if (!pass.m_Scissors[i].fixedResolution)
				{
					m_Scissors[i].bottom = height * pass.m_Scissors[i].scaler;
					m_Scissors[i].right = width * pass.m_Scissors[i].scaler;
				}
			}

			cmdList->RSSetViewports(m_Viewports.size(), m_Viewports.data());
			cmdList->RSSetScissorRects(m_Scissors.size(), m_Scissors.data());
		}
	}
}