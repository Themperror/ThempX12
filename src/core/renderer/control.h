#pragma once

#include "context.h"
#include "frame.h"
#include "device.h"
#include "gpu_resources.h"
#include "pass/mainpass.h"

#include <d3d12.h>
#include <memory>
namespace Themp
{
	namespace D3D
	{
		class Device;
		class Control
		{
		public:
			bool Init();
			void Stop();

			void BeginDraw();
			void EndDraw();

			MainPassHandle AddMainPass(const std::string& name);
			ComPtr<ID3D12GraphicsCommandList> GetImguiCmdList();
			ComPtr<ID3D12Device2> GetDevice() const;
			ComPtr<ID3D12Device5> GetDeviceRTX() const;
			const Context& GetContext() const;
			ComPtr<ID3D12CommandQueue> GetCommandQueue(D3D12_COMMAND_LIST_TYPE commandListType) const;
			GPU_Resources& GetResourceManager() const;
		private:
			Frame& GetCurrentBackbuffer();
			std::unique_ptr<Device> m_Device;
			std::unique_ptr<Context> m_Context;
			std::unique_ptr<GPU_Resources> m_GPU_Resources;
			ComPtr<ID3D12Fence> m_Fence;
			std::vector<Frame> m_Backbuffers;
			std::vector<uint64_t> m_FrameFenceValues;
			uint64_t m_FenceValue = 0;
			HANDLE m_FenceEvent;
			int m_CurrentBackBuffer = 0;

			ComPtr<ID3D12DescriptorHeap> m_ImguiSRVHeap;

			std::vector<MainPass> m_MainPasses;
		};
	}
}