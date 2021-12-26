#pragma once

#include "context.h"
#include "frame.h"
#include "device.h"
#include "gpu_resources.h"
#include "types.h"
#include "core/renderer/pipeline.h"
#include "core/renderer/object3d.h"

#include <d3d12.h>
#include <memory>
#include <vector>
namespace Themp
{
	class Resources;
	namespace D3D
	{
		class ShaderCompiler;
		class Device;
		class Control
		{
		public:
			struct Renderable
			{
				size_t object3D_ID;
				MeshData meshData;
			};
			struct RenderPass
			{
				Pipeline pipeline;
				std::vector<Renderable> renderables;
			};

		public:
			bool Init();
			void Stop();

			void PopulateRenderingGraph(Themp::Resources& resources);
			void BeginDraw();
			void EndDraw();
			void ResizeSwapchain(int width, int height);

			ComPtr<ID3D12GraphicsCommandList> GetImguiCmdList();
			ComPtr<ID3D12Device2> GetDevice() const;
			ComPtr<ID3D12Device5> GetDeviceRTX() const;
			const Context& GetContext() const;
			ComPtr<ID3D12CommandQueue> GetCommandQueue(D3D12_COMMAND_LIST_TYPE commandListType) const;
			GPU_Resources& GetResourceManager() const;
			void CreatePipelines(Themp::Resources& resources);
			const ShaderCompiler& GetShaderCompiler();
		private:
			Frame& GetCurrentBackbuffer();
			std::unique_ptr<Device> m_Device;
			std::unique_ptr<Context> m_Context;
			std::unique_ptr<GPU_Resources> m_GPU_Resources;
			ComPtr<ID3D12Fence> m_Fence;
			std::vector<Frame> m_Backbuffers;
			std::vector<uint64_t> m_FrameFenceValues;
			std::vector<RenderPass> m_Renderpasses;
			uint64_t m_FenceValue = 0;
			HANDLE m_FenceEvent;
			int m_CurrentBackBuffer = 0;

			ComPtr<ID3D12DescriptorHeap> m_ImguiSRVHeap;
		};
	}
}