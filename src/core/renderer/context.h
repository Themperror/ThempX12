#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <vector>
#include <cstdint>
#include <chrono>
#include "types.h"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

using namespace Microsoft::WRL;
namespace Themp
{
	namespace D3D
	{
		using RTVHeap = ComPtr<ID3D12DescriptorHeap>;
		class Texture;
		class Context
		{
		public:
			bool Init(ComPtr<ID3D12Device2> device);

			ComPtr<ID3D12Fence> CreateFence() const;
			HANDLE CreateEventHandle() const;
			uint64_t Signal(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, uint64_t & fenceValue) const;
			void WaitForFenceValue(ComPtr<ID3D12Fence> fence, uint64_t fenceValue, HANDLE fenceEvent, std::chrono::milliseconds duration = std::chrono::milliseconds::max()) const;
			void Flush(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, uint64_t & fenceValue, HANDLE fenceEvent) const;
			ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type) const;
			ComPtr<ID3D12GraphicsCommandList> CreateCommandList(ComPtr<ID3D12CommandAllocator> commandAllocator, D3D12_COMMAND_LIST_TYPE type) const;
			const Texture* GetBackBufferTexture(int idx)  const;

			void ResizeSwapchain(int width, int height);
			RTVHeap GetBackBufferHeap() const;

			ComPtr<IDXGISwapChain4> GetSwapChain() const;
			bool IsVSyncEnabled() const;
			bool SupportsFreeSync() const;
			void EnableVsync(bool enabled);
			ComPtr<ID3D12Device2> GetDevice() const;
		private:
			ComPtr<IDXGISwapChain4> CreateSwapChain(HWND hWnd, ComPtr<ID3D12CommandQueue> commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount);
			void SetupRenderTargetViews();

			ComPtr<IDXGISwapChain4> m_Swapchain;
			ComPtr<ID3D12Device2> m_Device;

			bool m_VSyncEnabled = false;
			bool m_SupportsFreeSync = false;
			int m_NumBackBuffers = 0;
			std::vector<Texture*> m_BackBuffers;
			DescriptorHeapTracker m_BackBufferHeapTracker;
			RTVHeap m_BackBufferHeap;
		};
	}
}