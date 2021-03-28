#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "types.h"
#include <bitset>
using namespace Microsoft::WRL;

namespace Themp
{
	namespace D3D
	{
		class Context;
		class GPU_Resources;
		class Texture
		{
		public:
			ComPtr<ID3D12Resource> GetResource(TEXTURE_TYPE type) const;
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const { return m_CPUHandle;  }
		private:

			void InitSRVTexture(ComPtr<ID3D12Resource> textureSource, const Context& device, const D3D::DescriptorHeapTracker& heapTracker);
			void InitDSVTexture(ComPtr<ID3D12Resource> textureSource, const Context& device, const D3D::DescriptorHeapTracker& heapTracker);
			void InitRTVTexture(ComPtr<ID3D12Resource> textureSource, const Context& device, const D3D::DescriptorHeapTracker& heapTracker);
			void InitUAVTexture(ComPtr<ID3D12Resource> textureSource, const Context& device, const D3D::DescriptorHeapTracker& heapTracker) {};

			ComPtr<ID3D12Resource> m_SRV;
			ComPtr<ID3D12Resource> m_UAV;
			ComPtr<ID3D12Resource> m_RTV;
			ComPtr<ID3D12Resource> m_DSV;
			std::bitset<4> m_InittedTypes;
			CD3DX12_CPU_DESCRIPTOR_HANDLE m_CPUHandle;
			uint32_t m_HeapIndex;
			friend class GPU_Resources;
		};
	}
}