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
namespace Themp
{
	namespace D3D 
	{

		GPU_Resources::GPU_Resources(const D3D::Device& device, uint32_t maxSRVs, uint32_t maxDSVs, uint32_t maxRTVs, uint32_t maxSamplers)
		{
			m_CB_SRV_UAV_Heap = CreateDescriptorHeap(device, Themp::D3D::DESCRIPTOR_HEAP_TYPE::CB_SRV_UAV, maxSRVs);
			int maxImGuiSRVs = Engine::s_SVars.GetSVarInt(SVar::iMaxImGuiSRVs);
			m_CB_SRV_UAV_Heap.usedSlots += maxImGuiSRVs;
			m_DSV_Heap = CreateDescriptorHeap(device, Themp::D3D::DESCRIPTOR_HEAP_TYPE::DSV, maxDSVs);
			m_RTV_Heap = CreateDescriptorHeap(device, Themp::D3D::DESCRIPTOR_HEAP_TYPE::RTV, maxRTVs);
			m_SAMPLER_Heap = CreateDescriptorHeap(device, Themp::D3D::DESCRIPTOR_HEAP_TYPE::SAMPLER, maxSamplers);
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
}