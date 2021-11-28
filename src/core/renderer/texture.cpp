#include "texture.h"
#include "engine.h"
#include "control.h"
#include "context.h"
#include "gpu_resources.h"
#include <d3dx12.h>
#include "util/break.h"
#include "util/print.h"
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
			Themp::Print("Attempted to get a resource type which this texture wasn't initialized with!");
			Themp::Break();
			return m_SRV;
		}

		void Texture::InitSRVTexture(ComPtr<ID3D12Resource> textureSource, ComPtr<ID3D12Device2> device, const D3D::DescriptorHeapTracker& heapTracker)
		{
			m_HeapIndex = heapTracker.usedSlots;

			UINT m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			D3D12_CPU_DESCRIPTOR_HANDLE heapStart = heapTracker.heap->GetCPUDescriptorHandleForHeapStart();
			m_CPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heapStart, m_HeapIndex, m_descriptorSize);

			D3D12_RESOURCE_DESC rDesc = textureSource->GetDesc();
			D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
			desc.Format = rDesc.Format;
			desc.Texture2D.MipLevels = rDesc.MipLevels;
			desc.Texture2D.MostDetailedMip = 0;
			desc.Texture2D.PlaneSlice = 0;
			desc.Texture2D.ResourceMinLODClamp = 0;

			m_InittedTypes[static_cast<size_t>(TEXTURE_TYPE::SRV)] = true;
			device->CreateShaderResourceView(textureSource.Get(), &desc, m_CPUHandle);
			m_SRV = textureSource;
		}

		void Texture::InitDSVTexture(ComPtr<ID3D12Resource> textureSource, ComPtr<ID3D12Device2> device, const D3D::DescriptorHeapTracker& heapTracker)
		{
			m_HeapIndex = heapTracker.usedSlots;

			UINT m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
			D3D12_CPU_DESCRIPTOR_HANDLE heapStart = heapTracker.heap->GetCPUDescriptorHandleForHeapStart();
			m_CPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heapStart, m_HeapIndex, m_descriptorSize);

			D3D12_RESOURCE_DESC rDesc = textureSource->GetDesc();
			D3D12_DEPTH_STENCIL_VIEW_DESC desc{};
			desc.Format = rDesc.Format;
			desc.Texture2D.MipSlice = 0;
			assert(rDesc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D);
			desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

			m_InittedTypes[static_cast<size_t>(TEXTURE_TYPE::DSV)] = true;
			device->CreateDepthStencilView(textureSource.Get(), &desc, m_CPUHandle);
			m_DSV = textureSource;
		}

		void Texture::InitRTVTexture(ComPtr<ID3D12Resource> textureSource, ComPtr<ID3D12Device2>device, const D3D::DescriptorHeapTracker& heapTracker)
		{
			m_HeapIndex = heapTracker.usedSlots;

			UINT m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			D3D12_CPU_DESCRIPTOR_HANDLE heapStart = heapTracker.heap->GetCPUDescriptorHandleForHeapStart();
			m_CPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heapStart, m_HeapIndex, m_descriptorSize);

			D3D12_RESOURCE_DESC rDesc = textureSource->GetDesc();
			D3D12_RENDER_TARGET_VIEW_DESC desc{};
			desc.Format = rDesc.Format;
			desc.Texture2D.MipSlice = 0;
			desc.Texture2D.PlaneSlice = 0;
			desc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2D;

			m_InittedTypes[static_cast<size_t>(TEXTURE_TYPE::RTV)] = true;
			device->CreateRenderTargetView(textureSource.Get(), &desc, m_CPUHandle);
			m_RTV = textureSource;
		}
	}
}