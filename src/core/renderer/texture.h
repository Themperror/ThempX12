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
		struct RenderPass;
		class Texture
		{
		public:
			/// <summary>
			/// SRV_PS: Read-only texture, used solely in Pixel Shader
			/// SRV_ALL: Ready-only texture, used somewhere in the entire pipeline (Vertex/PixelGeometry/Hull/Tesselation.. etc shader)
			/// SRVPlusDSVRead: Read-only Depth texture, used solely in Pixel Shader AND as Depth buffer with Depth write disabled)
			/// SRVPlusDSVRead: Read-only Depth texture, used somewhere in the entire pipeline (Vertex/PixelGeometry/Hull/Tesselation.. etc shader) AND as Depth buffer with Depth write disabled)
			/// DSV_Read: Depth buffer used in rasterisation with Depth write disabled)
			/// DSV_Write: Depth buffer used in rasterisation with Depth write enabled)
			/// RTV: Rendertarget used as OM target)
			/// Present: Rendertarget used as swapchain present)
			/// </summary>
			enum ResourceState 
			{
				SRV_PS = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 
				DSV_Read = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_READ,
				DSV_Write = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE,
				SRV_ALL = SRV_PS | D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
				SRVPlusDSVRead = SRV_PS | DSV_Read,
				SRV_ALLPlusDSVRead = SRV_ALL | DSV_Read,
				RTV = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET,
				Present = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT
			};


			ComPtr<ID3D12Resource> GetResource(TEXTURE_TYPE type) const;
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(TEXTURE_TYPE type) const;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(TEXTURE_TYPE type) const;
			void* GetHandleForImGui();
			bool HasType(TEXTURE_TYPE type) const;
			const D3D12_CLEAR_VALUE& GetClearValue() const { return m_ClearValue; }
			void SetClearValue(const D3D12_CLEAR_VALUE& val) { m_ClearValue = val; }
			void SetResourceState(RenderPass& usedPass, D3D::TEXTURE_TYPE type, ResourceState targetState);
		private:

			void InitSRVTexture(ComPtr<ID3D12Resource> textureSource, ComPtr<ID3D12Device2> device, const D3D::DescriptorHeapTracker& heapTracker);
			void InitDSVTexture(ComPtr<ID3D12Resource> textureSource, ComPtr<ID3D12Device2> device, const D3D::DescriptorHeapTracker& heapTracker);
			void InitRTVTexture(ComPtr<ID3D12Resource> textureSource, ComPtr<ID3D12Device2> device, const D3D::DescriptorHeapTracker& heapTracker);
			void InitUAVTexture(ComPtr<ID3D12Resource> textureSource, ComPtr<ID3D12Device2> device, const D3D::DescriptorHeapTracker& heapTracker) {};
			void ReInitSRVTexture(ComPtr<ID3D12Resource> textureSource, ComPtr<ID3D12Device2>device, const D3D::DescriptorHeapTracker& heapTracker);
			void ReInitDSVTexture(ComPtr<ID3D12Resource> textureSource, ComPtr<ID3D12Device2>device, const D3D::DescriptorHeapTracker& heapTracker);
			void ReInitRTVTexture(ComPtr<ID3D12Resource> textureSource, ComPtr<ID3D12Device2>device, const D3D::DescriptorHeapTracker& heapTracker);
			void ReInitUAVTexture(ComPtr<ID3D12Resource> textureSource, ComPtr<ID3D12Device2>device, const D3D::DescriptorHeapTracker& heapTracker) {};

			std::wstring CreateDebugName();

			std::bitset<4> m_InittedTypes;

			ResourceState m_CurrentResourceState;

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
			uint32_t m_SRVWidth = 0;
			uint32_t m_SRVHeight = 0;
			uint32_t m_SRVDepth = 0;
			D3D12_CLEAR_VALUE m_ClearValue;

			friend class GPU_Resources;
			friend class Resources;
			friend class Control;
		};
	}
}