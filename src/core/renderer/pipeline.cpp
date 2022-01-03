#include "core/renderer/pipeline.h"
#include "core/renderer/control.h"
#include "core/renderer/device.h"
#include "core/engine.h"

#include "core/util/svars.h"
#include "core/resources.h"
#include "core/util/print.h"
#include "core/util/break.h"
#include "core/util/stringUtils.h"
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
			assert(pass.m_RenderTargets.size() <= 8);
			for (int i = 0; i < pass.m_RenderTargets.size(); i++)
			{
				desc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;

				//0 is a special "swapchain" RTV
				if (pass.m_RenderTargets[i].rtv == 0)
				{
					desc.RTVFormats[numValidTargets] = Engine::instance->m_Renderer->GetContext().GetBackBufferTexture(0)->GetResource(D3D::TEXTURE_TYPE::RTV)->GetDesc().Format;
					//this will be overridden by the rendering every frame as this is variable
					m_RenderTargets.push_back({});
					numValidTargets++;
				}
				else if (pass.m_RenderTargets[i].rtv.IsValid()) 
				{
					Texture& tex = Themp::Engine::instance->m_Resources->Get(pass.m_RenderTargets[i].rtv);
					desc.RTVFormats[numValidTargets] = tex.GetResource(D3D::TEXTURE_TYPE::RTV)->GetDesc().Format;
					numValidTargets++;
					m_RenderTargets.push_back({ tex.GetCPUHandle(D3D::TEXTURE_TYPE::RTV), tex.GetClearValue()});
				}
			}
			desc.NumRenderTargets = numValidTargets;

			desc.DSVFormat = DXGI_FORMAT_UNKNOWN;
			if (pass.m_DepthTarget.dsv.IsValid())
			{
				Texture& tex = Themp::Engine::instance->m_Resources->Get(pass.m_DepthTarget.dsv);
				desc.DSVFormat = tex.GetResource(D3D::TEXTURE_TYPE::DSV)->GetDesc().Format;
				m_DepthTarget = { tex.GetCPUHandle(D3D::TEXTURE_TYPE::DSV), tex.GetClearValue() };
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
					m_Scissors[i].bottom = static_cast<LONG>(height * pass.m_Scissors[i].scaler);
					m_Scissors[i].right = static_cast<LONG>(width * pass.m_Scissors[i].scaler);
				}
			}

			std::vector<D3D12_INPUT_ELEMENT_DESC> iaLayouts;
			int byteOffSet = 0;
			int currentSlot = 0;

			//Instance data is always needed (for now)
			
			for (int i = 0; i < 4; i++)
			{
				D3D12_INPUT_ELEMENT_DESC& iaDesc = iaLayouts.emplace_back();
				iaDesc.AlignedByteOffset = byteOffSet;
				iaDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
				iaDesc.InputSlot = 0;
				iaDesc.InputSlotClass = D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
				iaDesc.SemanticName = "PER_INSTANCE_MATRIX";
				iaDesc.SemanticIndex = i;
				iaDesc.InstanceDataStepRate = 1;
				byteOffSet += 16;
			}

			byteOffSet = 0;
			if (subpass.NeedsPositionInfo)
			{
				D3D12_INPUT_ELEMENT_DESC& iaDesc = iaLayouts.emplace_back();
				iaDesc.AlignedByteOffset = byteOffSet;
				iaDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;
				iaDesc.InputSlot = 1;
				iaDesc.InputSlotClass = D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				iaDesc.SemanticName = "POSITION";
				byteOffSet += sizeof(DirectX::XMFLOAT3);
			}
			if (subpass.NeedsNormalInfo)
			{
				D3D12_INPUT_ELEMENT_DESC& normalDesc = iaLayouts.emplace_back();
				normalDesc.AlignedByteOffset = byteOffSet;
				normalDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;
				normalDesc.InputSlot = 2;
				normalDesc.InputSlotClass = D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				normalDesc.SemanticName = "NORMAL";
				byteOffSet += sizeof(DirectX::XMFLOAT3);
				
				D3D12_INPUT_ELEMENT_DESC& tangentDesc = iaLayouts.emplace_back();
				tangentDesc.AlignedByteOffset = byteOffSet;
				tangentDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;
				tangentDesc.InputSlot = 3;
				tangentDesc.InputSlotClass = D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				tangentDesc.SemanticName = "TANGENT";
				byteOffSet += sizeof(DirectX::XMFLOAT3);
				
				D3D12_INPUT_ELEMENT_DESC& bitangentDesc = iaLayouts.emplace_back();
				bitangentDesc.AlignedByteOffset = byteOffSet;
				bitangentDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;
				bitangentDesc.InputSlot = 4;
				bitangentDesc.InputSlotClass = D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				bitangentDesc.SemanticName = "BITANGENT";
				byteOffSet += sizeof(DirectX::XMFLOAT3);
			}
			if (subpass.NeedsUVInfo)
			{
				D3D12_INPUT_ELEMENT_DESC& iaDesc = iaLayouts.emplace_back();
				iaDesc.AlignedByteOffset = byteOffSet;
				iaDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT;
				iaDesc.InputSlot = 5;
				iaDesc.InputSlotClass = D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				iaDesc.SemanticName = "UV";
				byteOffSet += sizeof(DirectX::XMFLOAT2);
			}

			desc.InputLayout.NumElements = static_cast<UINT>(iaLayouts.size());
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

			auto device = Themp::Engine::instance->m_Renderer->GetDevice().GetDevice();
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
					
					device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
					desc.pRootSignature = m_RootSignature.Get();
				}
			}

			if (device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&m_Pipeline)) != S_OK)
			{
				Themp::Print("Failed to create pipeline state!");
				Themp::Break();
			}
			m_Pipeline->SetName(Themp::Util::ToWideString(pass.m_Name).c_str());
		}


		void Pipeline::SetTo(D3D::Frame& frame, RenderPass& renderPass)
		{
			auto cmdList = frame.GetCmdList();

			const auto& resources = Themp::Engine::instance->m_Resources;
			const Pass& pass = resources->Get(m_PassHandle);

			for (int i = 0; i < m_RenderTargets.size(); i++)
			{
				//0 is the special "current swapchain" rendertarget
				if (pass.m_RenderTargets[i].rtv == 0)
				{
					m_RenderTargets[i] = { frame.GetFrameBuffer().GetCPUHandle(D3D::TEXTURE_TYPE::RTV), {} };
					continue;
				}
				auto& tex = resources->Get(pass.m_RenderTargets[i].rtv);
				m_RenderTargets[i] = { tex.GetCPUHandle(D3D::TEXTURE_TYPE::RTV),tex.GetClearValue() };

				tex.SetResourceState(renderPass,TEXTURE_TYPE::RTV, Texture::ResourceState::RTV);
			}

			if (pass.m_DepthTarget.dsv.IsValid())
			{
				auto& tex = resources->Get(pass.m_DepthTarget.dsv);

				m_DepthTarget = { tex.GetCPUHandle(D3D::TEXTURE_TYPE::DSV), tex.GetClearValue() };
				D3D12_CLEAR_FLAGS flags = D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH;
				auto resource = tex.GetResource(TEXTURE_TYPE::DSV);
				if (resource->GetDesc().Format == DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT)
				{
					flags |= D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_STENCIL;
				}

				if (pass.m_DepthState.depthEnable)
				{
					if(pass.m_DepthState.depthWriteMask.mask != DepthWriteMask::ZERO)
					{
						tex.SetResourceState(renderPass, TEXTURE_TYPE::DSV, Texture::ResourceState::DSV_Write);
					}
					else
					{
						tex.SetResourceState(renderPass, TEXTURE_TYPE::DSV, Texture::ResourceState::DSV_Read);
					}
				}
			}
			

			cmdList->SetPipelineState(m_Pipeline.Get());
			cmdList->SetGraphicsRootSignature(m_RootSignature.Get());
			cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			D3D12_CPU_DESCRIPTOR_HANDLE RTHandles[8]{};
			for (int i = 0; i < m_RenderTargets.size(); i++)
			{
				RTHandles[i] = m_RenderTargets[i].first;
			}

			cmdList->OMSetRenderTargets(static_cast<UINT>(m_RenderTargets.size()), RTHandles, false, m_DepthTarget.first.ptr != 0 ? &m_DepthTarget.first : nullptr);

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
					m_Scissors[i].bottom = static_cast<LONG>(height * pass.m_Scissors[i].scaler);
					m_Scissors[i].right  = static_cast<LONG>(width * pass.m_Scissors[i].scaler);
				}
			}

			cmdList->RSSetViewports(static_cast<UINT>(m_Viewports.size()), m_Viewports.data());
			cmdList->RSSetScissorRects(static_cast<UINT>(m_Scissors.size()), m_Scissors.data());
		}

		void Pipeline::ClearTargets(D3D::Frame& frame)
		{
			auto cmdList = frame.GetCmdList();
			const auto& resources = Themp::Engine::instance->m_Resources;
			const Pass& pass = resources->Get(m_PassHandle);

			for (int i = 0; i < m_RenderTargets.size(); i++)
			{
				//0 is the special "current swapchain" rendertarget
				if (pass.m_RenderTargets[i].rtv == 0)
				{
					continue;
				}
				cmdList->ClearRenderTargetView(m_RenderTargets[i].first, m_RenderTargets[i].second.Color, 0, nullptr);
			}

			if (pass.m_DepthTarget.dsv.IsValid())
			{
				const auto& tex = resources->Get(pass.m_DepthTarget.dsv);

				D3D12_CLEAR_FLAGS flags = D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH;
				auto resource = tex.GetResource(TEXTURE_TYPE::DSV);
				if (resource->GetDesc().Format == DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT)
				{
					flags |= D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_STENCIL;
				}
				cmdList->ClearDepthStencilView(m_DepthTarget.first, flags, m_DepthTarget.second.DepthStencil.Depth, m_DepthTarget.second.DepthStencil.Stencil, 0, nullptr);
			}
		}

	}
}