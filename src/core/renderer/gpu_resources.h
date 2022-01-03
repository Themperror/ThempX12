#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include "types.h"
#include "texture.h"
#include <DirectXMath.h>

using namespace Microsoft::WRL;

namespace Themp
{
	class SceneObject;
}
namespace Themp::D3D
{
	class Device;
	class Model;
	struct RenderPass;
	class GPU_Resources
	{
	public:
		GPU_Resources(const D3D::Device& device);
	public:
		ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap(DESCRIPTOR_HEAP_TYPE type, uint32_t reservedSlots = 0);
		Texture& GetTextureFromResource(ComPtr<ID3D12Device2> device, ComPtr<ID3D12Resource> resource, TEXTURE_TYPE type, Texture::ResourceState initialState, int reusedIndex = -1, int* outResultingIndex = nullptr);
		ComPtr<ID3D12Resource> GetTextureResource(ComPtr<ID3D12Device2> device, const std::string& name, D3D12_RESOURCE_FLAGS flags, DXGI_FORMAT format, int mipCount, DXGI_SAMPLE_DESC multisample, int width, int height, int depth, D3D12_CLEAR_VALUE* optClearValue, D3D::TEXTURE_TYPE& outType, D3D::Texture::ResourceState& outState);

		void UpdateTransformsBufferView(const D3D::Device& device, D3D::RenderPass& pass, std::vector<SceneObject>& objects);
		void UpdateConstantBufferData(ConstantBufferHandle handle);
		void UpdateCameraConstantBuffer(D3D::ConstantBufferHandle handle, const CameraConstantBuffer& camData);
		void UpdateEngineConstantBuffer(D3D::ConstantBufferHandle handle, const EngineConstantBuffer& engineData);

		Model Test_GetAndAddRandomModel();

		int ReleaseTexture(Texture& tex);
		void UploadMeshStagingBuffers();
		MeshData AppendMeshToStagingBuffers(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

		ConstantBufferHandle CreateConstantBuffer(const D3D::Device& device, size_t size, ConstantBufferHandle reuseHandle = ConstantBufferHandle::Invalid);
		ConstantBufferHandle ReserveConstantBuffer();


		ConstantBufferData& Get(D3D::ConstantBufferHandle handle);

		ComPtr<ID3D12Resource> GetPositionVertexBuffer() const;
		ComPtr<ID3D12Resource> GetNormalVertexBuffer() const;
		ComPtr<ID3D12Resource> GetUVVertexBuffer() const;
		ComPtr<ID3D12Resource> GetIndexBuffer() const;

		const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const;
		const std::vector<D3D12_VERTEX_BUFFER_VIEW>& GetVertexBufferViews() const;

	private:
		struct MeshDataStage
		{
			std::vector<std::vector<Vertex>> vertexData;
			std::vector<std::vector<uint32_t>> indexData;
		};

		DescriptorHeapTracker CreateDescriptorHeap(const D3D::Device& device, DESCRIPTOR_HEAP_TYPE type, uint32_t amount);
		void CreateVertexBuffer(const D3D::Device& device, uint32_t maxVertexCount, bool copyOldBufferData);
		void CreateIndexBuffer(const D3D::Device& device, uint32_t maxIndexCount, bool copyOldBufferData);
		void UpdateIndexBufferView(const MeshData& tracker);
		uint32_t GetNextMeshID() { return m_LatestMeshID++; }

		//Texture& MakeTextureFromResource(const Context& device, DescriptorHeapTracker& heapTracker, ComPtr<ID3D12Resource> resource, TEXTURE_TYPE type);
		std::unordered_map<std::string, Texture*> m_FileTextures;
		std::vector< std::unique_ptr<Texture>> m_Textures;

		struct VertexCollectionBuffer
		{
			struct NormalData
			{
				DirectX::XMFLOAT3 normal;
				DirectX::XMFLOAT3 tangent;
				DirectX::XMFLOAT3 bitangent;
			};
		private:
			DirectX::XMFLOAT3* posData = nullptr;
			NormalData* normalData = nullptr;
			DirectX::XMFLOAT2* uvData = nullptr;
			D3D12_RANGE writtenRangePos{};
			D3D12_RANGE writtenRangeNormal{};
			D3D12_RANGE writtenRangeUV{};
		public:
			ComPtr<ID3D12Resource> positionBuffer;
			ComPtr<ID3D12Resource> normalBuffer;
			ComPtr<ID3D12Resource> uvBuffer;
			std::vector<D3D12_VERTEX_BUFFER_VIEW> m_VertexBufferViews;


		public:
			VertexCollectionBuffer() = default;
			VertexCollectionBuffer(ComPtr<ID3D12Resource> pos, ComPtr<ID3D12Resource> normal, ComPtr<ID3D12Resource> uv)
			{
				positionBuffer = pos;
				normalBuffer = normal;
				uvBuffer = uv;
				m_VertexBufferViews.resize(3);
			}

			void Map(size_t writeRangeBeginElements)
			{
				D3D12_RANGE read{};
				writtenRangePos.Begin = writeRangeBeginElements * sizeof(DirectX::XMFLOAT3);
				writtenRangeNormal.Begin = writeRangeBeginElements * sizeof(NormalData);
				writtenRangeUV.Begin = writeRangeBeginElements * sizeof(DirectX::XMFLOAT2);
				positionBuffer->Map(0, &read, (void**)&posData);
				normalBuffer->Map(0, &read, (void**)&normalData);
				uvBuffer->Map(0, &read, (void**)&uvData);
				
			}

			void Unmap()
			{
				positionBuffer->Unmap(0, &writtenRangePos);
				normalBuffer->Unmap(0, &writtenRangeNormal);
				uvBuffer->Unmap(0, &writtenRangeUV);

				posData = nullptr;
				normalData = nullptr;
				uvData = nullptr;
			}

			void UpdateVertexBufferViews(const MeshData& tracker)
			{
				m_VertexBufferViews[0].BufferLocation = positionBuffer->GetGPUVirtualAddress();
				m_VertexBufferViews[1].BufferLocation = normalBuffer->GetGPUVirtualAddress();
				m_VertexBufferViews[2].BufferLocation = uvBuffer->GetGPUVirtualAddress();

				m_VertexBufferViews[0].SizeInBytes = tracker.vertexCount * sizeof(DirectX::XMFLOAT3);
				m_VertexBufferViews[1].SizeInBytes = tracker.vertexCount * sizeof(NormalData);
				m_VertexBufferViews[2].SizeInBytes = tracker.vertexCount * sizeof(DirectX::XMFLOAT2);

				m_VertexBufferViews[0].StrideInBytes = sizeof(DirectX::XMFLOAT3);
				m_VertexBufferViews[1].StrideInBytes = sizeof(NormalData);
				m_VertexBufferViews[2].StrideInBytes = sizeof(DirectX::XMFLOAT2);
			}

			void SetVertex(int index, const Vertex& vertex)
			{
				posData[index] = vertex.position;
				writtenRangePos.End += sizeof(vertex.position);
				normalData[index].normal = vertex.normal;
				normalData[index].tangent = vertex.tangent;
				normalData[index].bitangent = vertex.bitangent;
				writtenRangeNormal.End += sizeof(vertex.normal) + sizeof(vertex.tangent) + sizeof(vertex.bitangent);
				uvData[index] = vertex.uv;
				writtenRangeUV.End += sizeof(vertex.uv);
			}
		};

		uint32_t m_MaxVertexBufferSize = 0;
		VertexCollectionBuffer m_MainVertexBuffers;

		std::vector<ConstantBufferData> m_ConstantBuffers;
		ComPtr<ID3D12Resource> m_InstanceTransformsBuffer;
		D3D12_VERTEX_BUFFER_VIEW m_InstanceTransformsView;
		
		uint32_t m_MaxIndexBufferSize = 0;
		ComPtr<ID3D12Resource> m_MainIndexBuffer;
		D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
		MeshData m_MeshBufferStageTracker;
		MeshData m_MeshBufferTracker;
		MeshDataStage m_MeshDataStage;
		MeshID m_LatestMeshID = 0;

		DescriptorHeapTracker m_CB_SRV_UAV_Heap;
		DescriptorHeapTracker m_RTV_Heap;
		DescriptorHeapTracker m_DSV_Heap;
		DescriptorHeapTracker m_SAMPLER_Heap;


	};
}