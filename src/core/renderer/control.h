#pragma once

#include "context.h"
#include "frame.h"
#include "device.h"
#include "gpu_resources.h"
#include "types.h"
#include "core/renderer/pipeline.h"

#include <lib/directxtk/ResourceUploadBatch.h>
#include <d3d12.h>
#include <memory>
#include <vector>
#include <unordered_map>
namespace Themp
{
	class Resources;
	namespace D3D
	{
		class ShaderCompiler;
		class Device;

		struct Renderable
		{
			std::vector<size_t> SceneObject_IDs;
			TransformBuffer m_PerInstanceTransforms;
			size_t numVisibleMeshes;
			MeshData meshData;
		};
		struct RenderPass
		{
			SubPassHandle relatedSubpassHandle;
			RenderPassHandle handle;
			Pipeline pipeline;
			std::vector<CD3DX12_RESOURCE_BARRIER> texturesToTransition;
			std::vector<std::pair<int, ConstantBufferHandle>> constantBuffers;
			std::unordered_map<D3D::MeshHandle, Renderable> renderables;
		};

		class Control
		{
		public:

		public:
			//Called right after construction
			bool Init();
			//Called before the rendering loop starts
			void Prepare();
			void Stop();


			void PopulateRenderingGraph(Themp::Resources& resources);
			void BeginDraw();
			void EndDraw();
			void ResizeSwapchain(int width, int height);

			ComPtr<ID3D12GraphicsCommandList> GetImguiCmdList();
			const Device& GetDevice() const;
			const Context& GetContext() const;
			GPU_Resources& GetResourceManager() const;
			void CreatePipelines(Themp::Resources& resources);

			const std::vector<RenderPass>& GetRenderPasses() const;
			RenderPass& GetRenderPass(D3D::RenderPassHandle handle);

			D3D::ConstantBufferHandle GetEngineConstantBuffer() const;
			D3D::ConstantBufferHandle GetCameraConstantBuffer() const;

			void AddForImGuiImageSRV(Texture* tex);
			DirectX::ResourceUploadBatch& GetResourceUploadBatch() { return *m_UploadBatch; }

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
			std::vector<Texture*> m_ImGuiSRVs;
			uint64_t m_FenceValue = 0;
			HANDLE m_FenceEvent;
			int m_CurrentBackBuffer = 0;

			std::unique_ptr<DirectX::ResourceUploadBatch> m_UploadBatch;
			ConstantBufferHandle m_CameraBuffer;
			ConstantBufferHandle m_EngineBuffer;

			ComPtr<ID3D12DescriptorHeap> m_ImguiSRVHeap;
		};
	}
}