#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <vector>

using namespace Microsoft::WRL;

namespace Themp::D3D
{
	static constexpr int InvalidHandle = -1;
	using MainPassHandle = int;
	using SubPassHandle = int;
	using MaterialHandle = int;
	using ShaderHandle = int;
	using TextureHandle = int;
	using ModelHandle = int;

	enum class DESCRIPTOR_HEAP_TYPE
	{
		CB_SRV_UAV,
		SAMPLER,
		RTV,
		DSV,
	};
	enum class TEXTURE_TYPE
	{
		SRV,
		UAV,
		RTV,
		DSV,
	};


	struct DescriptorHeapTracker
	{
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap;
		uint32_t usedSlots;
		uint32_t maxSlots;
	};

	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT3 tangent;
		DirectX::XMFLOAT3 bitangent;
		DirectX::XMFLOAT2 uv;
	};


	struct MeshData
	{
		uint32_t vertexIndex;
		uint32_t vertexCount;
		uint32_t indexIndex;
		uint32_t indexCount;


		void UpdateIndexBufferView(ComPtr<ID3D12Resource> indexBuffer)
		{
			indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
			indexBufferView.Format = DXGI_FORMAT::DXGI_FORMAT_R32_UINT;
			indexBufferView.SizeInBytes = indexCount * sizeof(uint32_t);
		}

		const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const
		{
			return indexBufferView;
		}


	private:
		D3D12_INDEX_BUFFER_VIEW indexBufferView;
	};
	struct ModelData
	{
		std::vector<MeshData> meshes;
	};
}
