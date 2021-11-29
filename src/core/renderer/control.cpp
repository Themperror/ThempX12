#include "renderer/control.h"
#include "util/print.h"
#include "util/svars.h"
#include "engine.h"
#include "renderer/texture.h"
#include "resources.h"
#include "shadercompiler.h"

#include <imgui.h>
#include <imgui/impl/imgui_impl_dx12.h>
using namespace Themp::D3D;


MeshData testMesh;

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


	ImGui_ImplDX12_Init(GetDevice().Get(), Engine::s_SVars.GetSVarInt(SVar::iNumBackBuffers), DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, m_ImguiSRVHeap.Get(), m_ImguiSRVHeap->GetCPUDescriptorHandleForHeapStart(), m_ImguiSRVHeap->GetGPUDescriptorHandleForHeapStart());

	s_ShaderCompiler.Init();


	testMesh = m_GPU_Resources->Test_GetAndAddRandomModel();
	return true;
}
void Control::Stop()
{
	m_FrameFenceValues[m_CurrentBackBuffer] = m_Context->Signal(m_Device->GetCmdQueue(), m_Fence, m_FenceValue);
	m_CurrentBackBuffer = m_Context->GetSwapChain()->GetCurrentBackBufferIndex();
	m_Context->WaitForFenceValue(m_Fence, m_FrameFenceValues[m_CurrentBackBuffer], m_FenceEvent);
}

void Control::CreatePipelines(Themp::Resources& resources, const std::vector<D3D::SubPass>& subPasses)
{
	for(int i = 0; i < subPasses.size(); i++)
	{
		auto& pipeline = m_Pipelines.emplace_back();
		pipeline.Init(subPasses[i]);
	}
}

void Control::BeginDraw()
{
	Frame& frame = GetCurrentBackbuffer();
	frame.Reset();
	auto CPUHandle = frame.GetFrameBuffer().GetCPUHandle();
	frame.GetCmdList()->OMSetRenderTargets(1, &CPUHandle, true, nullptr);

	if (ImGui::Begin("Renderer"))
	{
		static bool vsyncEnabled = true;
		m_Context->EnableVsync(vsyncEnabled);

		ImGui::Checkbox("VSync", &vsyncEnabled);
		ImGui::Text("Test");
	}
	ImGui::End();

	const Pipeline& pipeline = m_Pipelines[0];
	frame.GetCmdList()->SetPipelineState(pipeline.m_Pipeline.Get());
	frame.GetCmdList()->SetGraphicsRootSignature(pipeline.m_RootSignature.Get());
	frame.GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	frame.GetCmdList()->OMSetRenderTargets(1, &CPUHandle, true, &pipeline.m_DepthTarget);

	const auto& vertexBufferViews = m_GPU_Resources->GetVertexBufferViews();
	frame.GetCmdList()->IASetVertexBuffers(0, vertexBufferViews.size(), vertexBufferViews.data());
	frame.GetCmdList()->IASetIndexBuffer(&m_GPU_Resources->GetIndexBufferView());
	frame.GetCmdList()->DrawIndexedInstanced(testMesh.indexCount, 1, testMesh.indexIndex, testMesh.vertexIndex, 0);
	//frame.GetCmdList().Get()->SetDescriptorHeaps(1, m_ImguiSRVHeap.GetAddressOf());

}

void Control::EndDraw()
{
	Frame& frame = GetCurrentBackbuffer();
	frame.Present();

	m_FrameFenceValues[m_CurrentBackBuffer] = m_Context->Signal(m_Device->GetCmdQueue(), m_Fence, m_FenceValue);
	m_CurrentBackBuffer = m_Context->GetSwapChain()->GetCurrentBackBufferIndex();

	m_Context->WaitForFenceValue(m_Fence, m_FrameFenceValues[m_CurrentBackBuffer], m_FenceEvent);
}

Frame& Control::GetCurrentBackbuffer()
{
	return m_Backbuffers[m_CurrentBackBuffer];
}

ComPtr<ID3D12CommandQueue> Control::GetCommandQueue(D3D12_COMMAND_LIST_TYPE commandListType) const
{
	return m_Device->GetCmdQueue(commandListType);
}

ComPtr<ID3D12Device2> Control::GetDevice() const
{
	return m_Device->GetDevice();
}

ComPtr<ID3D12Device5> Control::GetDeviceRTX() const
{
	if (m_Device->SupportsRaytracing())
	{
		ComPtr<ID3D12Device5> dev5;
		HRESULT result = m_Device->GetDevice().As<ID3D12Device5>(&dev5);
		return dev5;
	}
	else
	{
		Themp::Print("Attempted to get device as RTX when we don't support this");
		return nullptr;
	}
}

const Context&  Control::GetContext() const
{
	return *m_Context;
}


ComPtr<ID3D12GraphicsCommandList> Control::GetImguiCmdList()
{
	return GetCurrentBackbuffer().GetCmdList();
}

const Themp::D3D::ShaderCompiler& Control::GetShaderCompiler()
{
	return s_ShaderCompiler;
}

GPU_Resources& Control::GetResourceManager() const
{
	return *m_GPU_Resources;
}