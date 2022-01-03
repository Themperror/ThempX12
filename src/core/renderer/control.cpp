#include "core/renderer/control.h"
#include "core/util/print.h"
#include "core/util/svars.h"
#include "core/engine.h"
#include "core/renderer/texture.h"
#include "core/resources.h"
#include "core/renderer/shadercompiler.h"
#include "core/components/sceneobject.h"
#include "core/resources.h"

#include <algorithm>


#include <lib/imgui/imgui.h>
#include <lib/imgui/impl/imgui_impl_dx12.h>


using namespace Themp::D3D;


Themp::D3D::ShaderCompiler s_ShaderCompiler;

bool Control::Init()
{
	m_Device = std::make_unique<Device>();
	if (!m_Device->Init())
	{
		return false;
	}

	m_GPU_Resources = std::make_unique<GPU_Resources>(*m_Device);

	int maxImGuiSRVs = Engine::s_SVars.GetSVarInt(SVar::iMaxImGuiSRVs);
	m_ImguiSRVHeap = m_GPU_Resources->GetDescriptorHeap(D3D::DESCRIPTOR_HEAP_TYPE::CB_SRV_UAV);

	m_Context = std::make_unique<Context>();
	if (!m_Context->Init(m_Device->GetDevice()))
	{
		return false;
	}

	int numBackBuffers = Engine::s_SVars.GetSVarInt(SVar::iNumBackBuffers);
	m_Backbuffers.resize(numBackBuffers);
	for (int i = 0; i < m_Backbuffers.size(); i++)
	{
		m_Backbuffers[i].Init(i, m_Context->GetBackBufferHeap());
	}

	m_FrameFenceValues.resize(numBackBuffers);

	m_Fence = m_Context->CreateFence();
	m_FenceEvent = m_Context->CreateEventHandle();

	m_Context->EnableVsync(Engine::s_SVars.GetSVarInt(SVar::iVSyncEnabled));


	ImGui_ImplDX12_Init(GetDevice().GetDevice().Get(), Engine::s_SVars.GetSVarInt(SVar::iNumBackBuffers), DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, m_ImguiSRVHeap.Get(), m_ImguiSRVHeap->GetCPUDescriptorHandleForHeapStart(), m_ImguiSRVHeap->GetGPUDescriptorHandleForHeapStart());

	s_ShaderCompiler.Init();


	m_EngineBuffer = m_GPU_Resources->CreateConstantBuffer(GetDevice(), sizeof(D3D::EngineConstantBuffer));
	m_CameraBuffer = m_GPU_Resources->CreateConstantBuffer(GetDevice(), sizeof(D3D::CameraConstantBuffer));

	return true;
}

void Control::Stop()
{
	m_FrameFenceValues[m_CurrentBackBuffer] = m_Context->Signal(m_Device->GetCmdQueue(), m_Fence, m_FenceValue);
	m_CurrentBackBuffer = m_Context->GetSwapChain()->GetCurrentBackBufferIndex();
	m_Context->WaitForFenceValue(m_Fence, m_FrameFenceValues[m_CurrentBackBuffer], m_FenceEvent);
}

void Control::CreatePipelines(Themp::Resources& resources)
{
	for(int i = 0; i < resources.GetAmountOfSubpasses(); i++)
	{
		auto& renderpass = m_Renderpasses.emplace_back();
		renderpass.pipeline.Init(resources.Get(SubPassHandle(i)));
	}

	std::sort(	
		m_Renderpasses.begin(), 
		m_Renderpasses.end(), 
		[&](const auto& a, const auto& b) 
		{ 
			return resources.Get(a.pipeline.GetPassHandle()).GetPriority() < resources.Get(b.pipeline.GetPassHandle()).GetPriority();
		}
	);

	for (int i = 0; i < m_Renderpasses.size(); i++)
	{
		m_Renderpasses[i].handle = i;
	}
}

void Control::PopulateRenderingGraph(Themp::Resources& resources)
{
	const auto& objs = resources.GetSceneObjects();
	for (const auto& obj : objs)
	{
		int meshID = 0;
		Model& model = resources.Get(obj.m_ModelHandle);
		for (int i = 0; i < obj.m_OverrideMaterials.size(); i++)
		{
			D3D::MaterialHandle materialOverride = obj.m_OverrideMaterials[i];
			if (materialOverride.IsValid())
			{
				const auto& material = resources.Get(materialOverride);
				for (const auto& subPassHandle : material.m_SubPasses)
				{
					for (auto& pass : m_Renderpasses)
					{
						if (pass.pipeline.GetPassHandle() == resources.Get(subPassHandle).pass)
						{
							auto it = pass.renderables.find(model.m_Meshes[i].m_MeshData.ID);
							if (it != pass.renderables.end())
							{
								Renderable& renderable = it->second;
								renderable.SceneObject_IDs.push_back(obj.m_ID);
							}
							else
							{
								Renderable& renderable = pass.renderables[model.m_Meshes[i].m_MeshData.ID];
								renderable.SceneObject_IDs.push_back(obj.m_ID);
								renderable.meshData = model.m_Meshes[i].m_MeshData;
							}
						}
					}
				}
			}
			else
			{
				Themp::Print("Encountered invalid material handle in mesh %i from object: %s", meshID, obj.m_Name.c_str());
			}
			meshID++;
		}
	}
}

void Control::ResizeSwapchain(int width, int height)
{
	for (int i = 0; i < m_FrameFenceValues.size(); i++)
	{
		m_FrameFenceValues[i] = m_Context->Signal(m_Device->GetCmdQueue(), m_Fence, m_FenceValue);
		m_Context->WaitForFenceValue(m_Fence, m_FrameFenceValues[i], m_FenceEvent);
	}

	m_Context->ResizeSwapchain(width, height);
	for (int i = 0; i < m_Backbuffers.size(); i++)
	{
		m_Backbuffers[i].RetrieveNewFrameBuffer();
	}
	m_CurrentBackBuffer = m_Context->GetSwapChain()->GetCurrentBackBufferIndex();
}

std::vector<CD3DX12_RESOURCE_BARRIER> SRVsForImGuiBarriers;
void Control::BeginDraw()
{
	Frame& frame = GetCurrentBackbuffer();
	frame.Reset();
	
	EngineConstantBuffer engineData{};
	engineData.time = Engine::instance->GetTimeSinceLaunch();
	engineData.screenWidth =  static_cast<float>(Engine::instance->s_SVars.GetSVarInt(SVar::iWindowWidth));
	engineData.screenHeight = static_cast<float>(Engine::instance->s_SVars.GetSVarInt(SVar::iWindowHeight));
	m_GPU_Resources->UpdateEngineConstantBuffer(m_EngineBuffer, engineData);	


	if (ImGui::Begin("Renderer"))
	{
		static bool vsyncEnabled = true;
		m_Context->EnableVsync(vsyncEnabled);

		ImGui::Checkbox("VSync", &vsyncEnabled);
		for (auto& target : Engine::instance->m_Resources->GetAllDSVs())
		{
			if (target.second.HasType(D3D::TEXTURE_TYPE::SRV))
			{
				ImGui::Image(target.second.GetHandleForImGui(), ImVec2(256, 256));
			}
		}
	}
	ImGui::End();

	const auto& vertexBufferViews = m_GPU_Resources->GetVertexBufferViews();
	frame.GetCmdList()->IASetVertexBuffers(1u, static_cast<UINT>(vertexBufferViews.size()), vertexBufferViews.data());
	frame.GetCmdList()->IASetIndexBuffer(&m_GPU_Resources->GetIndexBufferView());

	auto& sceneObjects = Engine::instance->m_Resources->GetSceneObjects();

	for(auto& renderPass : m_Renderpasses)
	{
#if _DEBUG
		std::string_view passNameSV = Engine::instance->m_Resources->Get(renderPass.pipeline.GetPassHandle()).GetName();
		frame.GetCmdList()->BeginEvent(1, passNameSV.data(), passNameSV.size() + 1);
#endif
		m_GPU_Resources->UpdateTransformsBufferView(*m_Device, renderPass, sceneObjects);
		renderPass.pipeline.SetTo(frame, renderPass);

		if (renderPass.texturesToTransition.size() > 0)
		{
			frame.GetCmdList()->ResourceBarrier(renderPass.texturesToTransition.size(), renderPass.texturesToTransition.data());
		}
		renderPass.texturesToTransition.clear();

		for (auto& CB : renderPass.constantBuffers)
		{
			if (CB.second.IsValid())
			{
				ConstantBufferData& CBData = m_GPU_Resources->Get(CB.second);
				if (CBData.dirty)
				{
					m_GPU_Resources->UpdateConstantBufferData(CB.second);
				}
				frame.GetCmdList()->SetGraphicsRootConstantBufferView(CB.first, CBData.buffer->GetGPUVirtualAddress());
			}
		}

		renderPass.pipeline.ClearTargets(frame);
		for(const auto& renderable : renderPass.renderables)
		{
			const auto& meshData = renderable.second.meshData;
			frame.GetCmdList()->IASetVertexBuffers(0, 1, &renderable.second.m_PerInstanceTransforms.view);
			frame.GetCmdList()->DrawIndexedInstanced(meshData.indexCount, renderable.second.numVisibleMeshes, meshData.indexIndex, meshData.vertexIndex, 0u);
		}
#if _DEBUG
		frame.GetCmdList()->EndEvent();
#endif
	}

#if _DEBUG
	frame.GetCmdList()->BeginEvent(1u, "ImGui", sizeof("ImGui"));
#endif
	auto CPUHandle = frame.GetFrameBuffer().GetCPUHandle(D3D::TEXTURE_TYPE::RTV);
	frame.GetCmdList()->OMSetRenderTargets(1, &CPUHandle, true, nullptr);

	for (auto texture : m_ImGuiSRVs)
	{
		if (texture->m_CurrentResourceState != Texture::ResourceState::SRV_PS)
		{
			SRVsForImGuiBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(
				texture->m_SRV.Get(),
				static_cast<D3D12_RESOURCE_STATES>(texture->m_CurrentResourceState), static_cast<D3D12_RESOURCE_STATES>(Texture::ResourceState::SRV_PS)));
			texture->m_CurrentResourceState = Texture::ResourceState::SRV_PS;
		}
	}

	if (SRVsForImGuiBarriers.size() > 0)
	{
		frame.GetCmdList()->ResourceBarrier(static_cast<UINT>(SRVsForImGuiBarriers.size()), SRVsForImGuiBarriers.data());
	}
	SRVsForImGuiBarriers.clear();
}

void Control::EndDraw()
{
	Frame& frame = GetCurrentBackbuffer();
#if _DEBUG
	frame.GetCmdList()->EndEvent();
#endif
	frame.Present();

	m_FrameFenceValues[m_CurrentBackBuffer] = m_Context->Signal(m_Device->GetCmdQueue(), m_Fence, m_FenceValue);
	m_CurrentBackBuffer = m_Context->GetSwapChain()->GetCurrentBackBufferIndex();

	m_Context->WaitForFenceValue(m_Fence, m_FrameFenceValues[m_CurrentBackBuffer], m_FenceEvent);
}

void Control::AddForImGuiImageSRV(Texture* tex)
{
	m_ImGuiSRVs.push_back(tex);
}

Frame& Control::GetCurrentBackbuffer()
{
	return m_Backbuffers[m_CurrentBackBuffer];
}

const Device& Control::GetDevice() const
{
	return *m_Device;
}


const Context&  Control::GetContext() const
{
	return *m_Context;
}


ComPtr<ID3D12GraphicsCommandList> Control::GetImguiCmdList()
{
	return GetCurrentBackbuffer().GetCmdList();
}


const std::vector<RenderPass>& Control::GetRenderPasses() const
{
	return m_Renderpasses;
}

RenderPass& Control::GetRenderPass(D3D::RenderPassHandle handle)
{
	return m_Renderpasses[handle.handle];
}



Themp::D3D::ConstantBufferHandle Control::GetEngineConstantBuffer() const
{
	return m_EngineBuffer;
}
Themp::D3D::ConstantBufferHandle Control::GetCameraConstantBuffer() const
{
	return m_CameraBuffer;
}

const Themp::D3D::ShaderCompiler& Control::GetShaderCompiler()
{
	return s_ShaderCompiler;
}

GPU_Resources& Control::GetResourceManager() const
{
	return *m_GPU_Resources;
}