#include "core/renderer/texture.h"
#include "core/engine.h"
#include "core/renderer/control.h"
#include "core/renderer/context.h"
#include "core/renderer/gpu_resources.h"
#include "core/util/break.h"
#include "core/util/print.h"
#include <lib/d3dx12.h>
#include <assert.h>

namespace Themp
{
	namespace D3D
	{

		ComPtr<ID3D12Resource> Texture::GetResource(TEXTURE_TYPE type) const
		{
			if (m_InittedTypes[static_cast<size_t>(type)])
			{
				switch (type)
				{
				case TEXTURE_TYPE::SRV:
					return m_SRV;
				case TEXTURE_TYPE::UAV:
					return m_UAV;
				case TEXTURE_TYPE::RTV:
					return m_RTV;
				case TEXTURE_TYPE::DSV:
					return m_DSV;
				}
			}
			//Themp::Print("Attempted to get a resource type which this texture wasn't initialized with!");
			//Themp::Break();
			return nullptr;
		}

		CD3DX12_CPU_DESCRIPTOR_HANDLE Texture::GetCPUHandle(TEXTURE_TYPE type) const
		{
			if (m_InittedTypes[static_cast<size_t>(type)])
			{
				switch (type)
				{
				default:
				case TEXTURE_TYPE::SRV:
					return m_SRVCPUHandle;
				case TEXTURE_TYPE::UAV:
					return m_UAVCPUHandle;
				case TEXTURE_TYPE::RTV:
					return m_RTVCPUHandle;
				case TEXTURE_TYPE::DSV:
					return m_DSVCPUHandle;
				}
			}
		}

		CD3DX12_GPU_DESCRIPTOR_HANDLE Texture::GetGPUHandle(TEXTURE_TYPE type) const
		{
			if (m_InittedTypes[static_cast<size_t>(type)])
			{
				switch (type)
				{
				default:
				case TEXTURE_TYPE::SRV:
					return m_SRVGPUHandle;
				case TEXTURE_TYPE::UAV:
					return m_UAVGPUHandle;
				case TEXTURE_TYPE::RTV:
					return m_RTVGPUHandle;
				case TEXTURE_TYPE::DSV:
					return m_DSVGPUHandle;
				}
			}
		}

		void Texture::InitSRVTexture(ComPtr<ID3D12Resource> textureSource, ComPtr<ID3D12Device2> device, const D3D::DescriptorHeapTracker& heapTracker)
		{
			m_HeapIndex = heapTracker.usedSlots;
			ReInitSRVTexture(textureSource, device, heapTracker);
			//UINT m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			//D3D12_CPU_DESCRIPTOR_HANDLE heapStart = heapTracker.heap->GetCPUDescriptorHandleForHeapStart();
			//D3D12_GPU_DESCRIPTOR_HANDLE heapStartGPU = heapTracker.heap->GetGPUDescriptorHandleForHeapStart();
			//m_SRVCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heapStart, m_HeapIndex, m_descriptorSize);
			//m_SRVGPUHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(heapStartGPU, m_HeapIndex, m_descriptorSize);
			//
			//D3D12_RESOURCE_DESC rDesc = textureSource->GetDesc();
			//D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
			//if (rDesc.Format == DXGI_FORMAT_D32_FLOAT)
			//{
			//	desc.Format = DXGI_FORMAT_R32_FLOAT;
			//}
			//else if (rDesc.Format == DXGI_FORMAT_D24_UNORM_S8_UINT)
			//{
			//	desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			//}
			//else
			//{
			//	desc.Format = rDesc.Format;
			//}
			//desc.ViewDimension = rDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D ? D3D12_SRV_DIMENSION_TEXTURE2D : D3D12_SRV_DIMENSION_UNKNOWN;
			//desc.Texture2D.MipLevels = rDesc.MipLevels;
			//desc.Texture2D.MostDetailedMip = 0;
			//desc.Texture2D.PlaneSlice = 0;
			//desc.Texture2D.ResourceMinLODClamp = 0;
			//desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			//
			//m_InittedTypes[static_cast<size_t>(TEXTURE_TYPE::SRV)] = true;
			//device->CreateShaderResourceView(textureSource.Get(), &desc, m_SRVCPUHandle);
			//m_SRV = textureSource;
			//m_SRV->SetName(L"Shader Resource View");
		}

		void Texture::InitDSVTexture(ComPtr<ID3D12Resource> textureSource, ComPtr<ID3D12Device2> device, const D3D::DescriptorHeapTracker& heapTracker)
		{
			m_HeapIndex = heapTracker.usedSlots;

			UINT m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
			D3D12_CPU_DESCRIPTOR_HANDLE heapStart = heapTracker.heap->GetCPUDescriptorHandleForHeapStart();
			m_DSVCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heapStart, m_HeapIndex, m_descriptorSize);

			D3D12_RESOURCE_DESC rDesc = textureSource->GetDesc();
			D3D12_DEPTH_STENCIL_VIEW_DESC desc{};
			desc.Format = rDesc.Format;
			desc.Texture2D.MipSlice = 0;
			assert(rDesc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D);
			desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

			m_InittedTypes[static_cast<size_t>(TEXTURE_TYPE::DSV)] = true;
			device->CreateDepthStencilView(textureSource.Get(), &desc, m_DSVCPUHandle);
			m_DSV = textureSource;
			m_DSV->SetName(L"Depth Target");
		}

		void Texture::InitRTVTexture(ComPtr<ID3D12Resource> textureSource, ComPtr<ID3D12Device2>device, const D3D::DescriptorHeapTracker& heapTracker)
		{
			m_HeapIndex = heapTracker.usedSlots;

			UINT m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			D3D12_CPU_DESCRIPTOR_HANDLE heapStart = heapTracker.heap->GetCPUDescriptorHandleForHeapStart();
			D3D12_GPU_DESCRIPTOR_HANDLE heapStartGPU = heapTracker.heap->GetGPUDescriptorHandleForHeapStart();
			m_RTVCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heapStart, m_HeapIndex, m_descriptorSize);
			m_RTVGPUHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(heapStartGPU, m_HeapIndex, m_descriptorSize);

			D3D12_RESOURCE_DESC rDesc = textureSource->GetDesc();
			D3D12_RENDER_TARGET_VIEW_DESC desc{};
			desc.Format = rDesc.Format;
			desc.Texture2D.MipSlice = 0;
			desc.Texture2D.PlaneSlice = 0;
			desc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2D;

			m_InittedTypes[static_cast<size_t>(TEXTURE_TYPE::RTV)] = true;
			device->CreateRenderTargetView(textureSource.Get(), &desc, m_RTVCPUHandle);
			m_RTV = textureSource;
			m_RTV->SetName(L"RenderTarget");
		}

		void Texture::ReInitSRVTexture(ComPtr<ID3D12Resource> textureSource, ComPtr<ID3D12Device2>device, const D3D::DescriptorHeapTracker& heapTracker) 
		{
			UINT m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			D3D12_CPU_DESCRIPTOR_HANDLE heapStart = heapTracker.heap->GetCPUDescriptorHandleForHeapStart();
			D3D12_GPU_DESCRIPTOR_HANDLE heapStartGPU = heapTracker.heap->GetGPUDescriptorHandleForHeapStart();
			m_SRVCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heapStart, m_HeapIndex, m_descriptorSize);
			m_SRVGPUHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(heapStartGPU, m_HeapIndex, m_descriptorSize);

			D3D12_RESOURCE_DESC rDesc = textureSource->GetDesc();
			D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
			if (rDesc.Format == DXGI_FORMAT_D32_FLOAT)
			{
				desc.Format = DXGI_FORMAT_R32_FLOAT;
			}
			else if (rDesc.Format == DXGI_FORMAT_D24_UNORM_S8_UINT)
			{
				desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			}
			else
			{
				desc.Format = rDesc.Format;
			}
			desc.ViewDimension = rDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D ? D3D12_SRV_DIMENSION_TEXTURE2D : D3D12_SRV_DIMENSION_UNKNOWN;
			desc.Texture2D.MipLevels = rDesc.MipLevels;
			desc.Texture2D.MostDetailedMip = 0;
			desc.Texture2D.PlaneSlice = 0;
			desc.Texture2D.ResourceMinLODClamp = 0;
			desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			m_InittedTypes[static_cast<size_t>(TEXTURE_TYPE::SRV)] = true;
			device->CreateShaderResourceView(textureSource.Get(), &desc, m_SRVCPUHandle);
			m_SRV = textureSource;
			m_SRV->SetName(L"Shader Resource View");
		}
		
		void Texture::ReInitDSVTexture(ComPtr<ID3D12Resource> textureSource, ComPtr<ID3D12Device2>device, const D3D::DescriptorHeapTracker& heapTracker)
		{
			UINT m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
			D3D12_CPU_DESCRIPTOR_HANDLE heapStart = heapTracker.heap->GetCPUDescriptorHandleForHeapStart();
			D3D12_GPU_DESCRIPTOR_HANDLE heapStartGPU = heapTracker.heap->GetGPUDescriptorHandleForHeapStart();
			m_DSVCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heapStart, m_HeapIndex, m_descriptorSize);
			m_DSVGPUHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(heapStartGPU, m_HeapIndex, m_descriptorSize);

			D3D12_RESOURCE_DESC rDesc = textureSource->GetDesc();
			D3D12_DEPTH_STENCIL_VIEW_DESC desc{};
			desc.Format = rDesc.Format;
			desc.Texture2D.MipSlice = 0;
			assert(rDesc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D);
			desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

			m_InittedTypes[static_cast<size_t>(TEXTURE_TYPE::DSV)] = true;
			device->CreateDepthStencilView(textureSource.Get(), &desc, m_DSVCPUHandle);
			m_DSV = textureSource;
			m_DSV->SetName(L"Depth Target");
		}

		void Texture::ReInitUAVTexture(ComPtr<ID3D12Resource> textureSource, ComPtr<ID3D12Device2>device, const D3D::DescriptorHeapTracker& heapTracker) {}
		void Texture::ReInitRTVTexture(ComPtr<ID3D12Resource> textureSource, ComPtr<ID3D12Device2>device, const D3D::DescriptorHeapTracker& heapTracker)
		{
			UINT m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			D3D12_CPU_DESCRIPTOR_HANDLE heapStart = heapTracker.heap->GetCPUDescriptorHandleForHeapStart();
			D3D12_GPU_DESCRIPTOR_HANDLE heapStartGPU = heapTracker.heap->GetGPUDescriptorHandleForHeapStart();
			m_RTVCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heapStart, m_HeapIndex, m_descriptorSize);
			m_RTVGPUHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(heapStartGPU, m_HeapIndex, m_descriptorSize);

			D3D12_RESOURCE_DESC rDesc = textureSource->GetDesc();
			D3D12_RENDER_TARGET_VIEW_DESC desc{};
			desc.Format = rDesc.Format;
			desc.Texture2D.MipSlice = 0;
			desc.Texture2D.PlaneSlice = 0;
			desc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2D;

			m_InittedTypes[static_cast<size_t>(TEXTURE_TYPE::RTV)] = true;
			device->CreateRenderTargetView(textureSource.Get(), &desc, m_RTVCPUHandle);
			m_RTV = textureSource;
			m_RTV->SetName(L"RenderTarget");
		}
	}
}