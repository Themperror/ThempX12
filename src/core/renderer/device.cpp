#include "device.h"
#include "util/print.h"
using namespace Themp::D3D;

bool Device::Init()
{
#if defined(_DEBUG)
	EnableDebug();
#endif

	m_Adapter = GetAdapter(D3D_FEATURE_LEVEL_12_1);
	if (m_Adapter == nullptr)
	{
		Themp::Print("Unable to find physical GPU with feature level 12_1, looking for 12_0 instead..");
		m_Adapter = GetAdapter(D3D_FEATURE_LEVEL_12_0);
		if (m_Adapter == nullptr)
		{
			Themp::Print("Unable to find physical GPU with feature level 12_0, D3D12 support is required! Giving up!");
			return false;
		}
		m_CurrentFeatureLevel = D3D_FEATURE_LEVEL_12_0;
	}
	else
	{
		m_CurrentFeatureLevel = D3D_FEATURE_LEVEL_12_1;
	}

	m_Device = CreateDevice();
	if (m_Device == nullptr)
	{
		Themp::Print("Unable create a D3D12 device!");
		return false;
	}

#if defined(_DEBUG)
	SetDebugFlags();
#endif

	GetCmdQueue(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);


	return true;
}

ComPtr<ID3D12Device2> Device::CreateDevice()
{
	ComPtr<ID3D12Device5> raytracingDevice;
	if (D3D12CreateDevice(m_Adapter.Get(), m_CurrentFeatureLevel, IID_PPV_ARGS(&raytracingDevice)) != S_OK)
	{
		ComPtr<ID3D12Device2> normalDevice;
		if (D3D12CreateDevice(m_Adapter.Get(), m_CurrentFeatureLevel, IID_PPV_ARGS(&normalDevice)) != S_OK)
		{
			//¯\_(ツ)_/¯
			return nullptr; 
		}
		Themp::Print("Non-raytracing device created!");
		m_RaytracingSupport = false;

		normalDevice->SetName(L"ID3D12Device2");
		return normalDevice;
	}
	Themp::Print("Raytracing Device created!");
	m_RaytracingSupport = true;
	raytracingDevice->SetName(L"ID3D12Device5");
	return raytracingDevice;
}

ComPtr<ID3D12Device2> Device::GetDevice() const
{
	return m_Device;
}

ComPtr<ID3D12CommandQueue> Device::GetCmdQueue(D3D12_COMMAND_LIST_TYPE CmdListType)
{
	if (m_CmdQueues[CmdListType] == nullptr)
	{
		m_CmdQueues[CmdListType] = CreateCommandQueue(CmdListType);
	}
	return m_CmdQueues[CmdListType];
}

ComPtr<ID3D12CommandQueue> Device::CreateCommandQueue(D3D12_COMMAND_LIST_TYPE CmdListType)
{
	ComPtr<ID3D12CommandQueue> d3d12CommandQueue;

	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = CmdListType;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	if (m_Device->CreateCommandQueue(&desc, IID_PPV_ARGS(&d3d12CommandQueue)) != S_OK)
	{
		Themp::Print("Failed creating the Command Queue");
		return nullptr;
	}

	d3d12CommandQueue->SetName(L"ID3D12CommandQueue");
	
	return d3d12CommandQueue;
}

D3D_FEATURE_LEVEL Themp::D3D::Device::GetFeatureLevel() const
{
	return m_CurrentFeatureLevel;
}

bool Themp::D3D::Device::SupportsRaytracing() const
{
	return m_RaytracingSupport;
}

ComPtr<IDXGIAdapter4> Device::GetAdapter(D3D_FEATURE_LEVEL requiredFeatureLevel)
{
	ComPtr<IDXGIFactory4> dxgiFactory;
	UINT createFactoryFlags = 0;
#if defined(_DEBUG)
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
	CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory));

	ComPtr<IDXGIAdapter1> dxgiAdapter1;
	ComPtr<IDXGIAdapter4> dxgiAdapter4;
	SIZE_T maxDedicatedVideoMemory = 0;
	DXGI_ADAPTER_DESC1 selectedAdapterDesc{};

	for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
		dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

		if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
		{
			//this is a software device (WARP)
			continue;
		}

		//Are we able to create the device (we're not actually creating it yet!)
		if (SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(), requiredFeatureLevel, __uuidof(ID3D12Device), nullptr)) &&
			dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
		{
			//this has more ram than previously enumerated devices, lets prefer this one.. (usually has better performance...)
			maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
			selectedAdapterDesc = dxgiAdapterDesc1;
			dxgiAdapter1.As(&dxgiAdapter4);
		}
	}

	Themp::Print("Selecting adapter: [VRAM: %lliMB] [Desc: %S]", selectedAdapterDesc.DedicatedVideoMemory / 1024 / 1024, selectedAdapterDesc.Description);
	return dxgiAdapter4;
}



ComPtr<ID3D12DescriptorHeap> Device::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors) const
{
	ComPtr<ID3D12DescriptorHeap> descriptorHeap;

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};

	desc.NumDescriptors = numDescriptors;
	desc.Type = type;
	if (type == D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || type == D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
	{
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	}

	if (m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)) != S_OK)
	{
		Themp::Print("Failed to create descriptor heap of type: %i", type);
		return nullptr;
	}

	descriptorHeap->SetName(L"ID3D12DescriptorHeap");
	return descriptorHeap;
}


#if defined(_DEBUG)
void Device::EnableDebug()
{
	D3D12GetDebugInterface(IID_PPV_ARGS(&m_DebugInterface));
	m_DebugInterface->EnableDebugLayer();
	
}

void Device::SetDebugFlags()
{
	ComPtr<ID3D12InfoQueue> pInfoQueue;
	if (SUCCEEDED(m_Device.As(&pInfoQueue)))
	{
		//Set debugging flags so we break when doing something horribly wrong
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

		// Suppress whole categories of messages
		//D3D12_MESSAGE_CATEGORY Categories[] = {};

		// Suppress messages based on their severity level
		D3D12_MESSAGE_SEVERITY Severities[] =
		{
			D3D12_MESSAGE_SEVERITY_INFO
		};

		// Suppress individual messages by their ID
		D3D12_MESSAGE_ID DenyIds[] = {
			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
		};

		D3D12_INFO_QUEUE_FILTER NewFilter = {};
		//NewFilter.DenyList.NumCategories = _countof(Categories);
		//NewFilter.DenyList.pCategoryList = Categories;
		NewFilter.DenyList.NumSeverities = _countof(Severities);
		NewFilter.DenyList.pSeverityList = Severities;
		NewFilter.DenyList.NumIDs = _countof(DenyIds);
		NewFilter.DenyList.pIDList = DenyIds;

		pInfoQueue->PushStorageFilter(&NewFilter);
	}
}

#endif