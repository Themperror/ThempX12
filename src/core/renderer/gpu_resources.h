#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include "types.h"
#include "texture.h"
#include <DirectXMath.h>
using namespace Microsoft::WRL;

namespace Themp::D3D
{
	class Device;
	class GPU_Resources
	{
	public:
		GPU_Resources(const D3D::Device& device);
	public:
		ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap(DESCRIPTOR_HEAP_TYPE type, uint32_t reservedSlots = 0);
		Texture& GetTextureFromResource(ComPtr<ID3D12Device2> device, ComPtr<ID3D12Resource> resource, TEXTURE_TYPE type);
		ComPtr<ID3D12Resource> GetTextureResource(ComPtr<ID3D12Device2> device, const std::wstring& name, D3D12_RESOURCE_FLAGS flags, DXGI_FORMAT format, int mipCount, DXGI_SAMPLE_DESC multisample, int width, int height, int depth, D3D12_CLEAR_VALUE* optClearValue, D3D::TEXTURE_TYPE& outType);

		MeshData Test_GetAndAddRandomModel();

		void UploadMeshStagingBuffers();

		ComPtr<ID3D12Resource> GetPositionVertexBuffer();
		ComPtr<ID3D12Resource> GetNormalVertexBuffer();
		ComPtr<ID3D12Resource> GetUVVertexBuffer();
		ComPtr<ID3D12Resource> GetIndexBuffer();

	private:
		struct MeshDataStage
		{
			std::vector<std::vector<Vertex>> vertexData;
			std::vector<std::vector<uint32_t>> indexData;
		};

		MeshData AppendMeshToStagingBuffers(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		DescriptorHeapTracker CreateDescriptorHeap(const D3D::Device& device, DESCRIPTOR_HEAP_TYPE type, uint32_t amount);
		void CreateVertexBuffer(const D3D::Device& device);
		void CreateIndexBuffer(const D3D::Device& device);



		//Texture& MakeTextureFromResource(const Context& device, DescriptorHeapTracker& heapTracker, ComPtr<ID3D12Resource> resource, TEXTURE_TYPE type);
		std::unordered_map<std::string, Texture*> m_FileTextures;
		std::vector< std::unique_ptr<Texture>> m_Textures;
		std::unordered_map<std::string, ModelData> m_Models;

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

		public:
			ComPtr<ID3D12Resource> positionBuffer;
			ComPtr<ID3D12Resource> normalBuffer;
			ComPtr<ID3D12Resource> uvBuffer;


		public:
			VertexCollectionBuffer() = default;
			VertexCollectionBuffer(ComPtr<ID3D12Resource> pos, ComPtr<ID3D12Resource> normal, ComPtr<ID3D12Resource> uv)
			{
				positionBuffer = pos;
				normalBuffer = normal;
				uvBuffer = uv;
			}

			void Map()
			{
				D3D12_RANGE read{};
				positionBuffer->Map(0, &read, (void**)&posData);
				normalBuffer->Map(0, &read, (void**)&normalData);
				uvBuffer->Map(0, &read, (void**)&uvData);
			}

			void Unmap(const D3D12_RANGE& written)
			{
				positionBuffer->Unmap(0, &written);
				normalBuffer->Unmap(0, &written);
				uvBuffer->Unmap(0, &written);

				posData = nullptr;
				normalData = nullptr;
				uvData = nullptr;
			}

			void SetVertex(int index, const Vertex& vertex)
			{
				posData[index] = vertex.position;
				normalData[index].normal = vertex.normal;
				normalData[index].tangent = vertex.tangent;
				normalData[index].bitangent = vertex.bitangent;
				uvData[index] = vertex.uv;
			}
		};

		VertexCollectionBuffer m_MainVertexBuffers;
		ComPtr<ID3D12Resource> m_MainIndexBuffer;
		MeshData m_MeshBufferStageTracker;
		MeshData m_MeshBufferTracker;
		MeshDataStage m_MeshDataStage;

		DescriptorHeapTracker m_CB_SRV_UAV_Heap;
		DescriptorHeapTracker m_RTV_Heap;
		DescriptorHeapTracker m_DSV_Heap;
		DescriptorHeapTracker m_SAMPLER_Heap;


	};
}