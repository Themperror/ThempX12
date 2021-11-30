#include "gpu_resources.h"
#include "engine.h"
#include "context.h"
#include "control.h"
#include "device.h"
#include "texture.h"
#include "util/print.h"
#include "util/break.h"
#include "util/svars.h"

#include <assert.h>
namespace Themp::D3D
{
	GPU_Resources::GPU_Resources(const D3D::Device& device)
	{
		int maxNumSRV = Engine::s_SVars.GetSVarInt(SVar::iMaxSRVs);
		int maxNumRTV = Engine::s_SVars.GetSVarInt(SVar::iMaxRTVs);
		int maxNumDSV = Engine::s_SVars.GetSVarInt(SVar::iMaxDSVs);
		int maxNumSamplers = Engine::s_SVars.GetSVarInt(SVar::iMaxSamplers);
		int maxImGuiSRVs = Engine::s_SVars.GetSVarInt(SVar::iMaxImGuiSRVs);

		m_CB_SRV_UAV_Heap = CreateDescriptorHeap(device, Themp::D3D::DESCRIPTOR_HEAP_TYPE::CB_SRV_UAV, maxNumSRV);
		m_CB_SRV_UAV_Heap.usedSlots += maxImGuiSRVs;
		m_DSV_Heap = CreateDescriptorHeap(device, Themp::D3D::DESCRIPTOR_HEAP_TYPE::DSV, maxNumDSV);
		m_RTV_Heap = CreateDescriptorHeap(device, Themp::D3D::DESCRIPTOR_HEAP_TYPE::RTV, maxNumRTV);
		m_SAMPLER_Heap = CreateDescriptorHeap(device, Themp::D3D::DESCRIPTOR_HEAP_TYPE::SAMPLER, maxNumSamplers);

		CreateVertexBuffer(device);
		CreateIndexBuffer(device);
	}

	void GPU_Resources::CreateVertexBuffer(const D3D::Device& device)
	{
		int maxVertices = Engine::s_SVars.GetSVarInt(SVar::iMaxVerticesInBuffer);
		D3D12_HEAP_PROPERTIES props{};
		props.Type = D3D12_HEAP_TYPE_UPLOAD;
		props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ComPtr<ID3D12Resource> positionBuffer;
		ComPtr<ID3D12Resource> normalBuffer;
		ComPtr<ID3D12Resource> uvBuffer;

		{
			desc.Width = maxVertices * sizeof(DirectX::XMFLOAT3);
			if (device.GetDevice()->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&positionBuffer)) < 0)
			{
				Themp::Print("Failed to create Position Vertex Buffer!");
				Themp::Break();
			}
			positionBuffer->SetName(L"PositionVertexBuffer");
		}

		{
			desc.Width = maxVertices * sizeof(VertexCollectionBuffer::NormalData);
			if (device.GetDevice()->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&normalBuffer)) < 0)
			{
				Themp::Print("Failed to create Normal Vertex Buffer!");
				Themp::Break();
			}
			normalBuffer->SetName(L"NormalVertexBuffer");
		}

		{
			desc.Width = maxVertices * sizeof(DirectX::XMFLOAT2);
			if (device.GetDevice()->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&uvBuffer)) < 0)
			{
				Themp::Print("Failed to create UV Vertex Buffer!");
				Themp::Break();
			}
			uvBuffer->SetName(L"UVVertexBuffer");
		}

		m_MainVertexBuffers = VertexCollectionBuffer(positionBuffer, normalBuffer, uvBuffer);
	}
	void GPU_Resources::CreateIndexBuffer(const D3D::Device& device)
	{
		int maxIndices = Engine::s_SVars.GetSVarInt(SVar::iMaxIndicesInBuffer);
		D3D12_HEAP_PROPERTIES props{};
		props.Type = D3D12_HEAP_TYPE_UPLOAD;
		props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Width = maxIndices * sizeof(uint32_t);
		if (desc.Width >= std::numeric_limits<uint32_t>::max())
		{
			Themp::Print("Index buffer size exceeds 32-bit size limit, cannot be used as index buffer due to D3D12_INDEX_BUFFER_VIEW taking UINTS!");
			Themp::Break();
		}
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		if (device.GetDevice()->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&m_MainIndexBuffer)) < 0)
		{
			Themp::Print("Failed to create Index Buffer!");
			Themp::Break();
		}
		m_MainIndexBuffer->SetName(L"MainIndexBuffer");
	}

	MeshData GPU_Resources::Test_GetAndAddRandomModel()
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;


		//0   1

        //2   3
		vertices.push_back(Vertex{ {-0.33f,  0.33f, 1.0f}, {0,0,1}, {0,0,0}, {0,0,0}, {0,0} });
		vertices.push_back(Vertex{ { 0.33f,  0.33f, 1.0f}, {0,0,1}, {0,0,0}, {0,0,0}, {1,0} });
		vertices.push_back(Vertex{ {-0.33f, -0.33f, 1.0f}, {0,0,1}, {0,0,0}, {0,0,0}, {0,1} });
		vertices.push_back(Vertex{ { 0.33f, -0.33f, 1.0f}, {0,0,1}, {0,0,0}, {0,0,0}, {1,1} });

		indices.push_back(0);
		indices.push_back(1);
		indices.push_back(2);
		indices.push_back(1);
		indices.push_back(3);
		indices.push_back(2);

		return AppendMeshToStagingBuffers(vertices, indices);

	}

	MeshData GPU_Resources::AppendMeshToStagingBuffers(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
	{
		int maxVertices = Engine::s_SVars.GetSVarInt(SVar::iMaxVerticesInBuffer);
		int maxIndices = Engine::s_SVars.GetSVarInt(SVar::iMaxIndicesInBuffer);
		if (static_cast<size_t>(m_MeshBufferTracker.indexCount) + static_cast<size_t>(m_MeshBufferStageTracker.indexCount) + indices.size() >= maxIndices)
		{
			Themp::Print("Ran out of free indices!!");
			Themp::Break();
		}
		if (static_cast<size_t>(m_MeshBufferTracker.vertexCount) + static_cast<size_t>(m_MeshBufferStageTracker.vertexCount) + vertices.size() >= maxVertices)
		{
			Themp::Print("Ran out of free vertices!!");
			Themp::Break();
		}

		m_MeshDataStage.indexData.push_back(indices);
		m_MeshDataStage.vertexData.push_back(vertices);

		m_MeshBufferStageTracker.indexIndex = m_MeshBufferStageTracker.indexCount;
		m_MeshBufferStageTracker.vertexIndex = m_MeshBufferStageTracker.vertexCount;
		m_MeshBufferStageTracker.indexCount += static_cast<uint32_t>(indices.size());
		m_MeshBufferStageTracker.vertexCount += static_cast<uint32_t>(vertices.size());
		return m_MeshBufferStageTracker;
	}

	void GPU_Resources::UploadMeshStagingBuffers()
	{
		if (m_MeshDataStage.indexData.size() > 0)
		{
			const D3D12_RANGE readRange{};
			D3D12_RANGE writeRange{};
			writeRange.Begin = m_MeshBufferTracker.indexIndex * sizeof(uint32_t);
			writeRange.End = writeRange.Begin;
				
			char* indexData;
			m_MainIndexBuffer->Map(0, &readRange, (void**)&indexData);
			for (auto& indices : m_MeshDataStage.indexData)
			{
				//write to the offset of our indexData
				//example: our buffer is 10 big and we only want to write from 5 to 8
				//we get our begin address (indexData) and offset it by our current element (writeRange.End)
				size_t dataSize = sizeof(uint32_t) * indices.size();
				memcpy(indexData + writeRange.End, indices.data(), dataSize);
				writeRange.End += dataSize;

				m_MeshBufferTracker.indexIndex = m_MeshBufferTracker.indexCount;
				size_t totalSize = m_MeshBufferTracker.indexCount;
				m_MeshBufferTracker.indexCount += static_cast<uint32_t>(indices.size());
			}
			m_MainIndexBuffer->Unmap(0, &writeRange);

			UpdateIndexBufferView(m_MeshBufferTracker);

			assert(m_MeshBufferTracker.indexCount == m_MeshBufferStageTracker.indexCount && m_MeshBufferTracker.indexIndex == m_MeshBufferStageTracker.indexIndex);
			m_MeshDataStage.indexData.clear();
		}

		if (m_MeshDataStage.vertexData.size() > 0)
		{
			const D3D12_RANGE readRange{};
			D3D12_RANGE writeRange{};
			writeRange.Begin = m_MeshBufferTracker.vertexIndex * sizeof(uint32_t);
			writeRange.End = writeRange.Begin;
;
			m_MainVertexBuffers.Map();
			for (auto& vertices : m_MeshDataStage.vertexData)
			{
				for (int i = 0; i < vertices.size(); i++)
				{
					m_MainVertexBuffers.SetVertex(m_MeshBufferTracker.vertexCount + i, vertices[i]);
				}
				size_t dataSize = sizeof(Vertex) * vertices.size();
				writeRange.End += dataSize;

				m_MeshBufferTracker.vertexIndex = m_MeshBufferTracker.vertexCount;
				m_MeshBufferTracker.vertexCount += static_cast<uint32_t>(vertices.size());
			}
			m_MainVertexBuffers.Unmap(writeRange);

			m_MainVertexBuffers.UpdateVertexBufferViews(m_MeshBufferTracker);

			assert(m_MeshBufferTracker.vertexCount == m_MeshBufferStageTracker.vertexCount && m_MeshBufferTracker.vertexIndex == m_MeshBufferStageTracker.vertexIndex);
			m_MeshDataStage.vertexData.clear();
		}
	}

	ComPtr<ID3D12Resource> GPU_Resources::GetPositionVertexBuffer() const
	{
		return m_MainVertexBuffers.positionBuffer;
	}

	ComPtr<ID3D12Resource> GPU_Resources::GetNormalVertexBuffer() const
	{
		return m_MainVertexBuffers.normalBuffer;
	}

	ComPtr<ID3D12Resource> GPU_Resources::GetUVVertexBuffer() const
	{
		return m_MainVertexBuffers.uvBuffer;
	}

	ComPtr<ID3D12Resource> GPU_Resources::GetIndexBuffer() const
	{
		return m_MainIndexBuffer;
	}

	void GPU_Resources::UpdateIndexBufferView(const MeshData& tracker)
	{
		m_IndexBufferView.BufferLocation = m_MainIndexBuffer->GetGPUVirtualAddress();
		m_IndexBufferView.Format = DXGI_FORMAT::DXGI_FORMAT_R32_UINT;
		m_IndexBufferView.SizeInBytes = tracker.indexCount * sizeof(uint32_t);
	}

	const D3D12_INDEX_BUFFER_VIEW& GPU_Resources::GetIndexBufferView() const
	{
		return m_IndexBufferView;
	}

	const std::vector<D3D12_VERTEX_BUFFER_VIEW>& GPU_Resources::GetVertexBufferViews() const
	{
		return m_MainVertexBuffers.m_VertexBufferViews;
	}



	D3D12_DESCRIPTOR_HEAP_TYPE GetDescriptorHeapType(DESCRIPTOR_HEAP_TYPE type)
	{
		switch (type)
		{
		case DESCRIPTOR_HEAP_TYPE::CB_SRV_UAV:
			return D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			break;
		case DESCRIPTOR_HEAP_TYPE::SAMPLER:
			return D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
			break;
		case DESCRIPTOR_HEAP_TYPE::RTV:
			return D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			break;
		case DESCRIPTOR_HEAP_TYPE::DSV:
			return D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			break;
		}
		assert(type >= DESCRIPTOR_HEAP_TYPE::CB_SRV_UAV && type <= DESCRIPTOR_HEAP_TYPE::DSV);
		return D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	}


	ComPtr<ID3D12Resource> GPU_Resources::GetTextureResource(ComPtr<ID3D12Device2> device, const std::wstring& name, D3D12_RESOURCE_FLAGS flags, DXGI_FORMAT format, int mipCount, DXGI_SAMPLE_DESC multisample, int width, int height, int depth, D3D12_CLEAR_VALUE* optClearValue, TEXTURE_TYPE& outType)
	{
		outType = flags == D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE ? D3D::TEXTURE_TYPE::SRV :
			(flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) ? D3D::TEXTURE_TYPE::DSV :
			(flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) ? D3D::TEXTURE_TYPE::RTV : D3D::TEXTURE_TYPE::SRV;

		D3D12_HEAP_PROPERTIES props{};
		props.Type = D3D12_HEAP_TYPE_DEFAULT;

		/* requires custom heaps 
		//set unknown for CB/SRV/UAV types, and NA for all others
		props.CPUPageProperty = outType == D3D::TEXTURE_TYPE::SRV ? D3D12_CPU_PAGE_PROPERTY_UNKNOWN : D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
		//RTV/DSV types are never read by CPU, put them in pool L1 (GPU); see: https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_memory_pool
		props.MemoryPoolPreference = outType == D3D::TEXTURE_TYPE::SRV ? D3D12_MEMORY_POOL_UNKNOWN : D3D12_MEMORY_POOL_L1;
		*/
		props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		DescriptorHeapTracker& heapTracker = outType == D3D::TEXTURE_TYPE::SRV ? m_CB_SRV_UAV_Heap :
			outType == D3D::TEXTURE_TYPE::DSV ? m_DSV_Heap :
			outType == D3D::TEXTURE_TYPE::RTV ? m_RTV_Heap : m_CB_SRV_UAV_Heap;

		if (heapTracker.usedSlots + 1 >= heapTracker.maxSlots)
		{
			Themp::Print("Ran out of space in DescriptorHeap for texture: [%S]", name.c_str());
			Themp::Break();
			return nullptr;
		}
		heapTracker.usedSlots++;

		D3D12_RESOURCE_DESC desc{};
		desc.Dimension = depth > 0 ? D3D12_RESOURCE_DIMENSION_TEXTURE3D : height > 0 ? D3D12_RESOURCE_DIMENSION_TEXTURE2D : D3D12_RESOURCE_DIMENSION_TEXTURE1D;
		desc.Width = 1;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		if (depth > 0)
		{
			desc.DepthOrArraySize = depth;
			if (width <= 0 || height <= 0)
			{
				Themp::Print("Width and Height needs to be at least > 0 when setting depth");
				Themp::Break();
				return nullptr;
			}
		}
		else if (height > 0)
		{
			desc.Height = height;
			if (width <= 0 )
			{
				Themp::Print("Width needs to be at least > 0 when setting height");
				Themp::Break();
				return nullptr;
			}
		}
		else if (width <= 0)
		{
			Themp::Print("Width needs to be at least > 0");
			Themp::Break();
			return nullptr;
		}
		desc.Width = width;

		desc.MipLevels = mipCount;
		desc.Format = format;
		desc.SampleDesc = multisample;
		if (desc.SampleDesc.Count <= 0)
		{
			Themp::Print("SampleDesc.Count (multisample) needs to be at least > 0");
			Themp::Break();
			return nullptr;
		}

		desc.Layout = outType == D3D::TEXTURE_TYPE::SRV ? D3D12_TEXTURE_LAYOUT_ROW_MAJOR : D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags = flags;

		ComPtr<ID3D12Resource> resource;
		D3D12_RESOURCE_STATES initialState = outType == D3D::TEXTURE_TYPE::SRV ? D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON : outType == D3D::TEXTURE_TYPE::DSV ? D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE : D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET;

		D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE;// outType == D3D::TEXTURE_TYPE::SRV ? D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES : D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;
		if (device->CreateCommittedResource(&props, heapFlags, &desc, initialState, optClearValue, IID_PPV_ARGS(&resource)) < 0)
		{
			Themp::Print("Failed to create texture resource!");
			Themp::Break();
			return nullptr;
		}
		resource->SetName(name.c_str());
		return resource;
	}
	Texture& GPU_Resources::GetTextureFromResource(ComPtr<ID3D12Device2> device, ComPtr<ID3D12Resource> resource, TEXTURE_TYPE type)
	{
		auto& tex = m_Textures.emplace_back(std::make_unique<Texture>());

		DescriptorHeapTracker& heapTracker = type == TEXTURE_TYPE::SRV ? m_CB_SRV_UAV_Heap :
											type == TEXTURE_TYPE::DSV ? m_DSV_Heap :
											type == TEXTURE_TYPE::RTV ? m_RTV_Heap : m_CB_SRV_UAV_Heap;

		if (heapTracker.usedSlots+1 < heapTracker.maxSlots)
		{
			switch (type)
			{
			case TEXTURE_TYPE::SRV:
				tex->InitSRVTexture(resource, device, heapTracker);
				break;
			case TEXTURE_TYPE::DSV:
				tex->InitDSVTexture(resource, device, heapTracker);
				break;
			case TEXTURE_TYPE::RTV:
				tex->InitRTVTexture(resource, device, heapTracker);
				break;
			case TEXTURE_TYPE::UAV:
				tex->InitUAVTexture(resource, device, heapTracker);
				break;
			}
			heapTracker.usedSlots++;
		}
		else
		{
			Themp::Print("Ran out of descriptor slots!");
			Themp::Break();
		}
		return *tex;
	}



	ComPtr<ID3D12DescriptorHeap> GPU_Resources::GetDescriptorHeap(DESCRIPTOR_HEAP_TYPE type, uint32_t reservedSlots)
	{
		switch (type)
		{
		case DESCRIPTOR_HEAP_TYPE::CB_SRV_UAV:
			m_CB_SRV_UAV_Heap.usedSlots += reservedSlots;
			return m_CB_SRV_UAV_Heap.heap;
			break;
		case DESCRIPTOR_HEAP_TYPE::DSV:
			m_DSV_Heap.usedSlots += reservedSlots;
			return m_DSV_Heap.heap;
			break;
		case DESCRIPTOR_HEAP_TYPE::RTV:
			m_RTV_Heap.usedSlots += reservedSlots;
			return m_RTV_Heap.heap;
			break;
		case DESCRIPTOR_HEAP_TYPE::SAMPLER:
			m_SAMPLER_Heap.usedSlots += reservedSlots;
			return m_SAMPLER_Heap.heap;
			break;
		default:
			Themp::Print("Attempted to get descriptor heap of undefined type!");
			Themp::Break();
			//this should never happen unless we're doing stupid int to enum casts
			m_CB_SRV_UAV_Heap.usedSlots += reservedSlots;
			return m_CB_SRV_UAV_Heap.heap;
			break;
		}
	}

	DescriptorHeapTracker GPU_Resources::CreateDescriptorHeap(const D3D::Device& device, DESCRIPTOR_HEAP_TYPE type, uint32_t amount)
	{
		ComPtr<ID3D12DescriptorHeap> descriptorHeap;
		switch (type)
		{
		case DESCRIPTOR_HEAP_TYPE::CB_SRV_UAV:
			descriptorHeap = device.CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, amount);
			break;
		case DESCRIPTOR_HEAP_TYPE::DSV:
			descriptorHeap = device.CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV, amount);
			break;
		case DESCRIPTOR_HEAP_TYPE::RTV:
			descriptorHeap = device.CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV, amount);
			break;
		case DESCRIPTOR_HEAP_TYPE::SAMPLER:
			descriptorHeap = device.CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, amount);
			break;
		default:
			Themp::Print("Attempted to create descriptor heap of undefined type!");
			descriptorHeap = nullptr;
			Themp::Break();
			break;
		}
		DescriptorHeapTracker tracker{};
		tracker.heap = descriptorHeap;
		tracker.usedSlots = 0;
		tracker.maxSlots = amount;
		return tracker;
	}


	//const Texture& MakeEmptyTexture(TEXTURE_TYPE type)
	//{
	//	
	//}
	//
	//const Texture& MakeTextureFromFile(const std::string path, TEXTURE_TYPE type)
	//{
	//
	//}
}