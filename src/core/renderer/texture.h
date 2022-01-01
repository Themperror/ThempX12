#pragma once

#include <d3d12.h>
#include <lib/d3dx12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "types.h"
#include <bitset>
using namespace Microsoft::WRL;

namespace Themp
{
	namespace D3D
	{
		class GPU_Resources;
		class Texture
		{
		public:
			ComPtr<ID3D12Resource> GetResource(TEXTURE_TYPE type) const;
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(TEXTURE_TYPE type) const;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(TEXTURE_TYPE type) const;
			bool HasType(TEXTURE_TYPE type) const;
			const D3D12_CLEAR_VALUE& GetClearValue() const { return m_ClearValue; }
			void SetClearValue(const D3D12_CLEAR_VALUE& val) { m_ClearValue = val; }
		private:

			void InitSRVTexture(ComPtr<ID3D12Resource> textureSource, ComPtr<ID3D12Device2> device, const D3D::DescriptorHeapTracker& heapTracker);
			void InitDSVTexture(ComPtr<ID3D12Resource> textureSource, ComPtr<ID3D12Device2> device, const D3D::DescriptorHeapTracker& heapTracker);
			void InitRTVTexture(ComPtr<ID3D12Resource> textureSource, ComPtr<ID3D12Device2> device, const D3D::DescriptorHeapTracker& heapTracker);
			void InitUAVTexture(ComPtr<ID3D12Resource> textureSource, ComPtr<ID3D12Device2> device, const D3D::DescriptorHeapTracker& heapTracker) {};
			void ReInitSRVTexture(ComPtr<ID3D12Resource> textureSource, ComPtr<ID3D12Device2>device, const D3D::DescriptorHeapTracker& heapTracker);
			void ReInitDSVTexture(ComPtr<ID3D12Resource> textureSource, ComPtr<ID3D12Device2>device, const D3D::DescriptorHeapTracker& heapTracker);
			void ReInitRTVTexture(ComPtr<ID3D12Resource> textureSource, ComPtr<ID3D12Device2>device, const D3D::DescriptorHeapTracker& heapTracker);
			void ReInitUAVTexture(ComPtr<ID3D12Resource> textureSource, ComPtr<ID3D12Device2>device, const D3D::DescriptorHeapTracker& heapTracker);


			std::bitset<4> m_InittedTypes;

			ComPtr<ID3D12Resource> m_SRV;
			ComPtr<ID3D12Resource> m_UAV;
			ComPtr<ID3D12Resource> m_RTV;
			ComPtr<ID3D12Resource> m_DSV;

			CD3DX12_CPU_DESCRIPTOR_HANDLE m_SRVCPUHandle;
			CD3DX12_CPU_DESCRIPTOR_HANDLE m_DSVCPUHandle;
			CD3DX12_CPU_DESCRIPTOR_HANDLE m_RTVCPUHandle;
			CD3DX12_CPU_DESCRIPTOR_HANDLE m_UAVCPUHandle;

			CD3DX12_GPU_DESCRIPTOR_HANDLE m_SRVGPUHandle;
			CD3DX12_GPU_DESCRIPTOR_HANDLE m_DSVGPUHandle;
			CD3DX12_GPU_DESCRIPTOR_HANDLE m_RTVGPUHandle;
			CD3DX12_GPU_DESCRIPTOR_HANDLE m_UAVGPUHandle;
			uint32_t m_HeapIndex = 0;

			bool m_DoScaleWithRes = false;
			float m_ScaleValue = 1.0;
			D3D12_CLEAR_VALUE m_ClearValue;

			friend class GPU_Resources;
			friend class Resources;
		};
	}
}