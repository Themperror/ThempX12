#include "core/renderer/context.h"
#include "core/engine.h"
#include "core/renderer/control.h"
#include "core/util/print.h"
#include "core/util/svars.h"
#include "core/renderer/texture.h"

using namespace Themp::D3D;

bool CheckTearingSupport();

bool Context::Init(ComPtr<ID3D12Device2> device)
{
	ComPtr<ID3D12CommandQueue> cmdQueue = Engine::instance->m_Renderer->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	m_Device = device;
	m_NumBackBuffers = Engine::s_SVars.GetSVarInt(SVar::iNumBackBuffers);
	m_BackBuffers.resize(m_NumBackBuffers);

	//Will fix this to not contain windows calls later..
	RECT rect{ 0,0,Engine::s_SVars.GetSVarInt(SVar::iWindowWidth), Engine::s_SVars.GetSVarInt(SVar::iWindowHeight) };
	GetClientRect(Engine::instance->m_Window, &rect);
	m_Swapchain = CreateSwapChain(Engine::instance->m_Window, cmdQueue, rect.right, rect.bottom, m_NumBackBuffers);
	if (m_Swapchain == nullptr)
	{
		return false;
	}
	m_BackBufferHeap = Engine::instance->m_Renderer->GetResourceManager().GetDescriptorHeap(D3D::DESCRIPTOR_HEAP_TYPE::RTV, m_NumBackBuffers);
	if (m_BackBufferHeap == nullptr)
	{
		return false;
	}
	SetupRenderTargetViews();

	return true;
}

ComPtr<ID3D12CommandAllocator> Context::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type) const
{
	ComPtr<ID3D12CommandAllocator> commandAllocator;
	if (m_Device->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator)) != S_OK)
	{
		Themp::Print("Failed to create command allocator!");
		return nullptr;
	}

	commandAllocator->SetName(L"ID3D12CommandAllocator");
	return commandAllocator;
}

ComPtr<ID3D12GraphicsCommandList> Context::CreateCommandList(ComPtr<ID3D12CommandAllocator> commandAllocator, D3D12_COMMAND_LIST_TYPE type) const
{
	ComPtr<ID3D12GraphicsCommandList> commandList;
	if (m_Device->CreateCommandList(0, type, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)) != S_OK)
	{
		Themp::Print("Failed to create command list!");
		return nullptr;
	}

	commandList->Close();
	commandList->SetName(L"ID3D12GraphicsCommandList");
	return commandList;

}

ComPtr<ID3D12Fence> Context::CreateFence() const
{
	ComPtr<ID3D12Fence> fence;

	if (m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)) != S_OK)
	{
		Themp::Print("Failed to create fence!");
		return nullptr;
	}
	fence->SetName(L"ID3D12Fence");
	return fence;
}

HANDLE Context::CreateEventHandle() const
{
	HANDLE fenceEvent;

	fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!fenceEvent)
	{
		Themp::Print("Failed to create fence event!");
		return nullptr;
	}

	return fenceEvent;
}

uint64_t Context::Signal(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, uint64_t& fenceValue) const
{
	uint64_t fenceValueForSignal = ++fenceValue;

	if (commandQueue->Signal(fence.Get(), fenceValueForSignal) != S_OK)
	{
		Themp::Print("Failed to signal GPU");
		return 0;
	}

	return fenceValueForSignal;
}

void Context::WaitForFenceValue(ComPtr<ID3D12Fence> fence, uint64_t fenceValue, HANDLE fenceEvent, std::chrono::milliseconds duration) const
{
	if (fence->GetCompletedValue() < fenceValue)
	{
		if (fence->SetEventOnCompletion(fenceValue, fenceEvent) == S_OK)
		{
			::WaitForSingleObject(fenceEvent, static_cast<DWORD>(duration.count()));
		}
	}
}

void Context::Flush(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, uint64_t& fenceValue, HANDLE fenceEvent) const
{
	uint64_t fenceValueForSignal = Signal(commandQueue, fence, fenceValue);
	WaitForFenceValue(fence, fenceValueForSignal, fenceEvent);
}

RTVHeap Context::GetBackBufferHeap() const
{
	return m_BackBufferHeap;
}

void Context::SetupRenderTargetViews()
{
	auto rtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_BackBufferHeap->GetCPUDescriptorHandleForHeapStart();
	
	for (int i = 0; i < m_NumBackBuffers; ++i)
	{
		ComPtr<ID3D12Resource> backBuffer;
		if (m_Swapchain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)) == S_OK)
		{
			m_BackBuffers[i] = &Engine::instance->m_Renderer->GetResourceManager().GetTextureFromResource(m_Device,  backBuffer.Get(), TEXTURE_TYPE::RTV);
			
			rtvHandle.ptr += rtvDescriptorSize;
		}
	}
}


ComPtr<IDXGISwapChain4> Context::GetSwapChain() const
{
	return m_Swapchain;
}

bool Context::SupportsFreeSync() const
{
	return m_SupportsFreeSync;
}

bool Context::IsVSyncEnabled() const
{
	return m_VSyncEnabled;
}
void Context::EnableVsync(bool enabled)
{
	m_VSyncEnabled = enabled;
}

const Texture* Context::GetBackBufferTexture(int idx) const
{
	return m_BackBuffers[idx];
}

ComPtr<ID3D12Device2> Context::GetDevice() const
{
	return m_Device;
}

ComPtr<IDXGISwapChain4> Context::CreateSwapChain(HWND hWnd, ComPtr<ID3D12CommandQueue> commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount)
{
	ComPtr<IDXGISwapChain4> dxgiSwapChain4;
	ComPtr<IDXGIFactory4> dxgiFactory4;
	UINT createFactoryFlags = 0;
#if defined(_DEBUG)
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
	if (CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)) != S_OK)
	{
		Themp::Print("Failed creating the DXGI factory");
		return nullptr;
	}


	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc = { 1, 0 };
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = bufferCount;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	// It is recommended to always allow tearing if tearing support is available.
	m_SupportsFreeSync = CheckTearingSupport();
	swapChainDesc.Flags = m_SupportsFreeSync ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	ComPtr<IDXGISwapChain1> swapChain1;
	if (dxgiFactory4->CreateSwapChainForHwnd(commandQueue.Get(), hWnd, &swapChainDesc, nullptr, nullptr, &swapChain1) != S_OK)
	{
		Themp::Print("Failed creating the swapchain!");
		return nullptr;
	}

	// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
	// will be handled manually.
	if (dxgiFactory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER) != S_OK)
	{
		//non-fatal
		Themp::Print("Couldn't disable ALT-ENTER for fullscreen, too bad!");
	}

	if (swapChain1.As<IDXGISwapChain4>(&dxgiSwapChain4) != S_OK)
	{
		Themp::Print("Couldn't cast IDXGISwapChain1 to IDXGISwapChain4!");
		return nullptr;
	}


	return dxgiSwapChain4;
}

bool CheckTearingSupport()
{
	BOOL allowTearing = FALSE;

	// Rather than create the DXGI 1.5 factory interface directly, we create the
	// DXGI 1.4 interface and query for the 1.5 interface. This is to enable the 
	// graphics debugging tools which will not support the 1.5 factory interface 
	// until a future update.
	ComPtr<IDXGIFactory4> factory4;
	if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
	{
		ComPtr<IDXGIFactory5> factory5;
		if (SUCCEEDED(factory4.As(&factory5)))
		{
			if (FAILED(factory5->CheckFeatureSupport(
				DXGI_FEATURE_PRESENT_ALLOW_TEARING,
				&allowTearing, sizeof(allowTearing))))
			{
				allowTearing = FALSE;
			}
		}
	}

	return allowTearing == TRUE;
}