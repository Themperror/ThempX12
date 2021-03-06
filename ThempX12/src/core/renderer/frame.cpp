#include "frame.h"
#include "engine.h"
#include "control.h"
#include <d3dx12.h>
#include "util/print.h"
using namespace Themp::D3D;

void Frame::Init(int bufferIndex, RTVHeap heap)
{
	auto device = Engine::instance->m_Renderer->GetDevice();
	
	const Context& context = Engine::instance->m_Renderer->GetContext();
	m_FrameBuffer = context.GetBackBufferTexture(bufferIndex);

	m_CommandAllocator = context.CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT);
	m_CommandList = context.CreateCommandList(m_CommandAllocator, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);
}

void Frame::Reset()
{
	m_CommandAllocator->Reset();
	m_CommandList->Reset(m_CommandAllocator.Get(),nullptr);
	
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_FrameBuffer->GetResource(TEXTURE_TYPE::RTV).Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	m_CommandList->ResourceBarrier(1, &barrier);
	FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_FrameBuffer->GetCPUHandle();

	m_CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	
}

void Frame::Present()
{
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_FrameBuffer->GetResource(TEXTURE_TYPE::RTV).Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_CommandList->ResourceBarrier(1, &barrier);

	if (m_CommandList->Close() == S_OK)
	{
		ID3D12CommandList* const commandLists[] = {
			m_CommandList.Get()
		};
		Engine::instance->m_Renderer->GetCommandQueue(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT)->ExecuteCommandLists(_countof(commandLists), commandLists);
	}
	else
	{
		Themp::Print("Failed closing the command list!, we'll probably crash or do something very wrong from here on..");
	}
	const Context& context = Engine::instance->m_Renderer->GetContext();
	UINT syncInterval = context.IsVSyncEnabled() ? 1 : 0;
	UINT presentFlags = context.SupportsFreeSync() && !context.IsVSyncEnabled() ? DXGI_PRESENT_ALLOW_TEARING : 0;
	if (context.GetSwapChain()->Present(syncInterval, presentFlags) != S_OK)
	{
		Themp::Print("Presenting failed, good luck..");
	}

}

ComPtr<ID3D12CommandAllocator> Frame::GetCommandAllocator() const
{
	return m_CommandAllocator;
}

const Texture& Frame::GetFrameBuffer() const
{
	return *m_FrameBuffer;
}

ComPtr<ID3D12GraphicsCommandList> Themp::D3D::Frame::GetCmdList() const
{
	return m_CommandList;
}
