#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <stdint.h>
using namespace Microsoft::WRL;
namespace Themp
{
	namespace D3D
	{
		class Device
		{
		public:
			bool Init();
			ComPtr<ID3D12Device2> GetDevice() const;
			ComPtr<ID3D12CommandQueue> GetCmdQueue(D3D12_COMMAND_LIST_TYPE CmdListType = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);
			ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors) const;
			D3D_FEATURE_LEVEL GetFeatureLevel() const;
			bool SupportsRaytracing() const;
		private:
			ComPtr<ID3D12CommandQueue> CreateCommandQueue(D3D12_COMMAND_LIST_TYPE CmdListType = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);
			ComPtr<ID3D12Device2> CreateDevice();
			ComPtr<IDXGIAdapter4> GetAdapter(D3D_FEATURE_LEVEL requiredFeatureLevel);
		public:

		private:
#if defined(_DEBUG)
			void EnableDebug();
			void SetDebugFlags();
			ComPtr<ID3D12Debug> m_DebugInterface;
#endif
			ComPtr<IDXGIAdapter4> m_Adapter;
			ComPtr<ID3D12Device2> m_Device;
			ComPtr<ID3D12CommandQueue> m_CmdQueues[D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS + 1]{}; //last value in the enum;
			D3D_FEATURE_LEVEL m_CurrentFeatureLevel{};
			bool m_RaytracingSupport = false;

		};
	}
}