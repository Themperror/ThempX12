#include "core/renderer/gpu_resources.h"
#include "core/engine.h"
#include "core/renderer/context.h"
#include "core/renderer/control.h"
#include "core/renderer/device.h"
#include "core/renderer/texture.h"
#include "core/util/print.h"
#include "core/util/break.h"
#include "core/util/svars.h"
#include "core/util/stringUtils.h"
#include "core/renderer/model.h"
#include "core/components/transform.h"
#include "core/components/sceneobject.h"

#include "core/resources.h"

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
	}

	void GPU_Resources::CreateVertexBuffer(const D3D::Device& device, uint32_t maxVertexCount, bool copyOldBufferData)
	{
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
			desc.Width = maxVertexCount * sizeof(DirectX::XMFLOAT3);
			if (device.GetDevice()->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&positionBuffer)) < 0)
			{
				Themp::Print("Failed to create Position Vertex Buffer!");
				Themp::Break();
			}
			positionBuffer->SetName(L"PositionVertexBuffer");
		}

		{
			desc.Width = maxVertexCount * sizeof(VertexCollectionBuffer::NormalData);
			if (device.GetDevice()->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&normalBuffer)) < 0)
			{
				Themp::Print("Failed to create Normal Vertex Buffer!");
				Themp::Break();
			}
			normalBuffer->SetName(L"NormalVertexBuffer");
		}

		{
			desc.Width = maxVertexCount * sizeof(DirectX::XMFLOAT2);
			if (device.GetDevice()->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&uvBuffer)) < 0)
			{
				Themp::Print("Failed to create UV Vertex Buffer!");
				Themp::Break();
			}
			uvBuffer->SetName(L"UVVertexBuffer");
		}

		if (copyOldBufferData)
		{
			auto CopyBuffer = [&](auto oldBuffer, auto newBuffer, size_t elementSize)
			{
				const D3D12_RANGE newDataReadRange{};
				D3D12_RANGE oldDataReadRange{};
				oldDataReadRange.Begin = 0;
				oldDataReadRange.End = m_MaxVertexBufferSize * elementSize;
				D3D12_RANGE writeRange{};
				writeRange.Begin = 0;
				writeRange.End = writeRange.Begin;

				char* newData;
				char* oldData;
				newBuffer->Map(0, &newDataReadRange, (void**)&newData);
				oldBuffer->Map(0, &oldDataReadRange, (void**)&oldData);

				memcpy(newData, oldData, oldDataReadRange.End);
				writeRange.End = oldDataReadRange.End;

				oldBuffer->Unmap(0, &writeRange);
				newBuffer->Unmap(0, &writeRange);
			};

			CopyBuffer(m_MainVertexBuffers.positionBuffer, positionBuffer, sizeof(DirectX::XMFLOAT3));
			CopyBuffer(m_MainVertexBuffers.normalBuffer, normalBuffer, sizeof(VertexCollectionBuffer::NormalData));
			CopyBuffer(m_MainVertexBuffers.uvBuffer, uvBuffer, sizeof(DirectX::XMFLOAT2));
			
			m_MainVertexBuffers.positionBuffer.Reset();
			m_MainVertexBuffers.normalBuffer.Reset();
			m_MainVertexBuffers.uvBuffer.Reset();
		}

		m_MainVertexBuffers = VertexCollectionBuffer(positionBuffer, normalBuffer, uvBuffer);
	}
	void GPU_Resources::CreateIndexBuffer(const D3D::Device& device, uint32_t maxIndexCount, bool copyOldBufferData)
	{
		D3D12_HEAP_PROPERTIES props{};
		props.Type = D3D12_HEAP_TYPE_UPLOAD;
		props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Width = (uint64_t)maxIndexCount * sizeof(uint32_t);
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

		ComPtr<ID3D12Resource> newIndexBuffer;

		if (device.GetDevice()->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&newIndexBuffer)) < 0)
		{
			Themp::Print("Failed to create Index Buffer!");
			Themp::Break();
		}
		newIndexBuffer->SetName(L"MainIndexBuffer");

		if (copyOldBufferData)
		{
			const D3D12_RANGE newDataReadRange{};
			D3D12_RANGE oldDataReadRange{};
			oldDataReadRange.Begin = 0;
			oldDataReadRange.End = m_MaxIndexBufferSize * sizeof(uint32_t);
			D3D12_RANGE writeRange{};
			writeRange.Begin = 0;
			writeRange.End = writeRange.Begin;

			char* newData;
			char* oldData;
			newIndexBuffer->Map(0, &newDataReadRange, (void**)&newData);
			m_MainIndexBuffer->Map(0, &oldDataReadRange, (void**)&oldData);

			memcpy(newData, oldData, oldDataReadRange.End);
			writeRange.End = oldDataReadRange.End;

			m_MainIndexBuffer->Unmap(0, &writeRange);
			newIndexBuffer->Unmap(0, &writeRange);
			m_MainIndexBuffer.Reset();
		}
		m_MainIndexBuffer = newIndexBuffer;
	}

	Model GPU_Resources::Test_GetAndAddRandomModel()
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

		Model model{};
		model.m_Meshes.emplace_back().m_MeshData = AppendMeshToStagingBuffers(vertices, indices);
		return model;

	}

	int GPU_Resources::ReleaseTexture(Texture& tex)
	{
		for (int i = 0; i < m_Textures.size(); i++)
		{
			if (m_Textures[i]->m_HeapIndex == tex.m_HeapIndex)
			{
				m_Textures[i]->m_RTV.Reset();
				m_Textures[i]->m_DSV.Reset();
				m_Textures[i]->m_SRV.Reset();
				m_Textures[i]->m_UAV.Reset();
				return i;
			}
		}
		return -1;
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
		MeshData meshData;
		meshData.ID = GetNextMeshID();
		meshData.indexCount = static_cast<uint32_t>(indices.size());
		meshData.indexIndex = m_MeshBufferStageTracker.indexCount;
		meshData.vertexCount = static_cast<uint32_t>(vertices.size());
		meshData.vertexIndex = m_MeshBufferStageTracker.vertexCount;


		m_MeshBufferStageTracker.indexIndex = m_MeshBufferStageTracker.indexCount;
		m_MeshBufferStageTracker.vertexIndex = m_MeshBufferStageTracker.vertexCount;
		m_MeshBufferStageTracker.indexCount += static_cast<uint32_t>(indices.size());
		m_MeshBufferStageTracker.vertexCount += static_cast<uint32_t>(vertices.size());
		return meshData;
	}


	ConstantBufferHandle GPU_Resources::CreateConstantBuffer(const D3D::Device& device, size_t size, ConstantBufferHandle reuseHandle)
	{
		auto& newBuffer = reuseHandle.IsValid() ? Get(reuseHandle) : m_ConstantBuffers.emplace_back();
		if (reuseHandle.IsValid())
		{
			//release in the case it wasn't null
			newBuffer.buffer.Reset();
		}
		else
		{
			newBuffer.data.resize(size);
		}

		D3D12_HEAP_PROPERTIES props{};
		props.Type = D3D12_HEAP_TYPE_UPLOAD;
		props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Width = size;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		if (device.GetDevice()->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&newBuffer.buffer)) < 0)
		{
			Themp::Print("Failed to create Constant Buffer!");
			Themp::Break();
		}
		newBuffer.buffer->SetName(L"ConstantBuffer");
		return m_ConstantBuffers.size()-1;
	}

	ConstantBufferHandle GPU_Resources::ReserveConstantBuffer()
	{
		auto& CB = m_ConstantBuffers.emplace_back();
		return m_ConstantBuffers.size()-1;
	}

	ConstantBufferData& GPU_Resources::Get(D3D::ConstantBufferHandle handle)
	{
		return m_ConstantBuffers[handle.handle];
	}

	void GPU_Resources::UpdateCameraConstantBuffer(D3D::ConstantBufferHandle handle, const CameraConstantBuffer& camData)
	{
		ConstantBufferData& CB = Get(handle);

		const D3D12_RANGE readRange{};
		D3D12_RANGE writeRange{};
		writeRange.Begin = 0;
		writeRange.End = writeRange.Begin;

		char* data;
		CB.buffer->Map(0, &readRange, (void**)&data);

		memcpy(data, &camData, sizeof(CameraConstantBuffer));
		writeRange.End += sizeof(CameraConstantBuffer);

		CB.buffer->Unmap(0, &writeRange);
	}
	void GPU_Resources::UpdateEngineConstantBuffer(D3D::ConstantBufferHandle handle, const EngineConstantBuffer& engineData)
	{
		ConstantBufferData& CB = Get(handle);

		const D3D12_RANGE readRange{};
		D3D12_RANGE writeRange{};
		writeRange.Begin = 0;
		writeRange.End = writeRange.Begin;

		char* data;
		CB.buffer->Map(0, &readRange, (void**)&data);

		memcpy(data, &engineData, sizeof(EngineConstantBuffer));
		writeRange.End += sizeof(EngineConstantBuffer);

		CB.buffer->Unmap(0, &writeRange);
	}

	void GPU_Resources::UpdateConstantBufferData(D3D::ConstantBufferHandle handle)
	{
		ConstantBufferData& CB = Get(handle);
		CB.dirty = false;

		if (CB.nextOffset > 0 && CB.buffer == nullptr)
		{
			//align on 16 bytes
			size_t size = CB.nextOffset + (16 - (CB.nextOffset % 16));
			CreateConstantBuffer(Themp::Engine::instance->m_Renderer->GetDevice(), size, handle);
		}
		else if (CB.nextOffset == 0)
		{
			return;
		}
		
		const D3D12_RANGE readRange{};
		D3D12_RANGE writeRange{};
		writeRange.Begin = 0;
		writeRange.End = writeRange.Begin;

		char* data;
		CB.buffer->Map(0, &readRange, (void**)&data);

		memcpy(data, CB.data.data(), CB.data.size());
		writeRange.End += CB.data.size();

		CB.buffer->Unmap(0, &writeRange);
	}

	void GPU_Resources::UploadMeshStagingBuffers()
	{
		if (m_MeshDataStage.indexData.size() > 0)
		{
			uint32_t totalIndices = 0;
			for (auto& indexData : m_MeshDataStage.indexData)
			{
				totalIndices += indexData.size();
			}
			if (totalIndices + m_MeshBufferTracker.indexCount >= m_MaxIndexBufferSize)
			{
				uint32_t newMax = (float)(totalIndices + m_MeshBufferTracker.indexCount) * 1.5f;
				if (m_MaxIndexBufferSize == 0)
				{
					CreateIndexBuffer(Themp::Engine::instance->m_Renderer->GetDevice(), newMax, false);
				}
				else
				{
					CreateIndexBuffer(Themp::Engine::instance->m_Renderer->GetDevice(), newMax, true);
				}
				m_MaxIndexBufferSize = newMax;
			}

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
			uint32_t totalVertices = 0;
			for (auto& vertexData : m_MeshDataStage.vertexData)
			{
				totalVertices += vertexData.size();
			}
			if (totalVertices + m_MeshBufferTracker.vertexCount >= m_MaxVertexBufferSize)
			{
				uint32_t newMax = (float)(totalVertices + m_MeshBufferTracker.vertexCount) * 1.5f;
				if (m_MaxVertexBufferSize == 0)
				{
					CreateVertexBuffer(Themp::Engine::instance->m_Renderer->GetDevice(), newMax, false);
				}
				else
				{
					CreateVertexBuffer(Themp::Engine::instance->m_Renderer->GetDevice(), newMax, true);
				}
				m_MaxVertexBufferSize = newMax;
			}

			m_MainVertexBuffers.Map(m_MeshBufferTracker.vertexIndex * sizeof(uint32_t));
			for (auto& vertices : m_MeshDataStage.vertexData)
			{
				for (int i = 0; i < vertices.size(); i++)
				{
					m_MainVertexBuffers.SetVertex(m_MeshBufferTracker.vertexCount + i, vertices[i]);
				}
				size_t dataSize = sizeof(Vertex) * vertices.size();

				m_MeshBufferTracker.vertexIndex = m_MeshBufferTracker.vertexCount;
				m_MeshBufferTracker.vertexCount += static_cast<uint32_t>(vertices.size());
			}
			m_MainVertexBuffers.Unmap();

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

	void GPU_Resources::UpdateTransformsBufferView(const D3D::Device& device, RenderPass& pass , std::vector<SceneObject>& objects)
	{
		auto& resources = *Themp::Engine::instance->m_Resources;
		size_t sizePerElement = sizeof(DirectX::XMFLOAT4X4);
		
		for (auto& renderable : pass.renderables)
		{
			if (renderable.second.SceneObject_IDs.size() > renderable.second.m_PerInstanceTransforms.maxTransformsInResource)
			{
				//we need to make a new buffer to contain the extra transforms
				auto& perInstanceTransforms = renderable.second.m_PerInstanceTransforms;

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

				
				perInstanceTransforms.maxTransformsInResource = renderable.second.SceneObject_IDs.size() * 2;

				//release our current resource if any
				perInstanceTransforms.transformsResource.Reset();

				desc.Width = perInstanceTransforms.maxTransformsInResource * sizeof(DirectX::XMFLOAT4X4);
				if (device.GetDevice()->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&perInstanceTransforms.transformsResource)) < 0)
				{
					Themp::Print("Failed to create PerInstance Transform Buffer!");
					Themp::Break();
				}
				perInstanceTransforms.transformsResource->SetName(L"PerInstanceTransformBuffer");
				
			}

			//Update the data
			{
				auto& perInstanceTransforms = renderable.second.m_PerInstanceTransforms;
				auto transformsResource = renderable.second.m_PerInstanceTransforms.transformsResource;

				//we can update / shrink the buffer
				const D3D12_RANGE readRange{};
				D3D12_RANGE writeRange{};
				writeRange.Begin = 0;
				writeRange.End = writeRange.Begin;

				renderable.second.numVisibleMeshes = 0;

				char* transformData = nullptr;
				transformsResource->Map(0, &readRange, (void**)&transformData);

				for (auto& objID : renderable.second.SceneObject_IDs)
				{
					if (!objects[objID].m_Visible) continue;

					renderable.second.numVisibleMeshes++;
					auto& obj = objects[objID];
					const auto& matrix = obj.m_Transform.GetModelMatrix();

					memcpy(transformData + writeRange.End, &matrix, sizePerElement);
					writeRange.End += sizePerElement;
				}
				transformsResource->Unmap(0, &writeRange);


				perInstanceTransforms.view.BufferLocation = transformsResource->GetGPUVirtualAddress();
				perInstanceTransforms.view.SizeInBytes = static_cast<UINT>(renderable.second.SceneObject_IDs.size() * sizePerElement);
				perInstanceTransforms.view.StrideInBytes = static_cast<UINT>(sizePerElement);
			}
		}
		

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


	ComPtr<ID3D12Resource> GPU_Resources::GetTextureResource(ComPtr<ID3D12Device2> device, const std::string& name, D3D12_RESOURCE_FLAGS flags, DXGI_FORMAT format, int mipCount, DXGI_SAMPLE_DESC multisample, int width, int height, int depth, D3D12_CLEAR_VALUE* optClearValue, TEXTURE_TYPE& outType, Texture::ResourceState& outState)
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
		outState = outType == D3D::TEXTURE_TYPE::SRV ? Texture::ResourceState::SRV_PS : outType == D3D::TEXTURE_TYPE::DSV ? Texture::ResourceState::DSV_Write : Texture::ResourceState::RTV;

		D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE;// outType == D3D::TEXTURE_TYPE::SRV ? D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES : D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;
		if (device->CreateCommittedResource(&props, heapFlags, &desc, static_cast<D3D12_RESOURCE_STATES>(outState), optClearValue, IID_PPV_ARGS(&resource)) < 0)
		{
			Themp::Print("Failed to create texture resource!");
			Themp::Break();
			return nullptr;
		}
		resource->SetName(Themp::Util::ToWideString(name).c_str());
		return resource;
	}
	Texture& GPU_Resources::GetTextureFromResource(ComPtr<ID3D12Device2> device, ComPtr<ID3D12Resource> resource, TEXTURE_TYPE type, Texture::ResourceState initialState, int reusedIndex, int* outResultingIndex)
	{
		auto& tex = reusedIndex != -1 ? m_Textures[reusedIndex] : m_Textures.emplace_back(std::make_unique<Texture>());
		if (outResultingIndex != nullptr)
		{
			*outResultingIndex = reusedIndex != -1 ? reusedIndex : static_cast<int>(m_Textures.size()-1ull);
		}
		DescriptorHeapTracker& heapTracker = type == TEXTURE_TYPE::SRV ? m_CB_SRV_UAV_Heap :
											type == TEXTURE_TYPE::DSV ? m_DSV_Heap :
											type == TEXTURE_TYPE::RTV ? m_RTV_Heap : m_CB_SRV_UAV_Heap;

		tex->m_CurrentResourceState = initialState;
		if(reusedIndex != -1)
		{
			switch (type)
			{
			case TEXTURE_TYPE::SRV:
				if (tex->m_InittedTypes[static_cast<int>(TEXTURE_TYPE::SRV)])
				{
					tex->ReInitSRVTexture(resource, device, heapTracker);
				}
				else
				{
					tex->InitSRVTexture(resource, device, heapTracker);
				}
				break;
			case TEXTURE_TYPE::DSV:
				tex->ReInitDSVTexture(resource, device, heapTracker);
				break;
			case TEXTURE_TYPE::RTV:
				tex->ReInitRTVTexture(resource, device, heapTracker);
				break;
			case TEXTURE_TYPE::UAV:
				tex->ReInitUAVTexture(resource, device, heapTracker);
				break;
			}
		}
		else
		{
			if (heapTracker.usedSlots + 1 < heapTracker.maxSlots)
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