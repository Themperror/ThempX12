#include "gpu_resources.h"
#include "engine.h"
#include "context.h"
#include "control.h"
#include "device.h"
#include "texture.h"
#include "util/print.h"
#include "util/break.h"
namespace Themp
{
	namespace D3D 
	{
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
		}

		DescriptorHeapTracker& GPU_Resources::GetDescriptorHeap(const D3D::Context& context, DESCRIPTOR_HEAP_TYPE type, uint32_t amount)
		{
			DescriptorHeapTracker& tracker = MakeDescriptorHeapTracker(type);
			ComPtr<ID3D12DescriptorHeap> heap = MakeDescriptorHeap(context, tracker, type, amount);
			return tracker;
		}

		Texture& GPU_Resources::GetTextureFromResource(const Context& context, DescriptorHeapTracker& heapTracker, ComPtr<ID3D12Resource> resource, TEXTURE_TYPE type)
		{
			auto& tex = m_Textures.emplace_back(std::make_unique<Texture>());
			if (heapTracker.usedSlots < heapTracker.maxSlots)
			{
				switch (type)
				{
				case TEXTURE_TYPE::SRV:
					tex->InitSRVTexture(resource, context, heapTracker);
					break;
				case TEXTURE_TYPE::DSV:
					tex->InitDSVTexture(resource, context, heapTracker);
					break;
				case TEXTURE_TYPE::RTV:
					tex->InitRTVTexture(resource, context, heapTracker);
					break;
				case TEXTURE_TYPE::UAV:
					tex->InitUAVTexture(resource, context, heapTracker);
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



		DescriptorHeapTracker& GPU_Resources::MakeDescriptorHeapTracker(DESCRIPTOR_HEAP_TYPE type)
		{
			switch (type)
			{
			case DESCRIPTOR_HEAP_TYPE::CB_SRV_UAV:
				return *m_CB_SRV_UAV_Heaps.emplace_back(std::make_unique<DescriptorHeapTracker>());
				break;
			case DESCRIPTOR_HEAP_TYPE::DSV:
				return *m_DSV_Heaps.emplace_back(std::make_unique<DescriptorHeapTracker>());
				break;
			case DESCRIPTOR_HEAP_TYPE::RTV:
				return *m_RTV_Heaps.emplace_back(std::make_unique<DescriptorHeapTracker>());
				break;
			case DESCRIPTOR_HEAP_TYPE::SAMPLER:
				return *m_SAMPLER_Heaps.emplace_back(std::make_unique<DescriptorHeapTracker>());
				break;
			default:
				Themp::Print("Attempted to get descriptor heap of undefined type!");
				Themp::Break();
				//return dummy reference?
				//if we don't break and handle it we'll certainly crash&burn
				//then again this should never happen unless we're doing stupid int to enum casts?
				break;
			}
		}

		ComPtr<ID3D12DescriptorHeap> GPU_Resources::MakeDescriptorHeap(const D3D::Context& context, DescriptorHeapTracker& tracker, DESCRIPTOR_HEAP_TYPE type, uint32_t amount)
		{
			ComPtr<ID3D12DescriptorHeap> descriptorHeap;
			switch (type)
			{
			case DESCRIPTOR_HEAP_TYPE::CB_SRV_UAV:
				descriptorHeap = context.CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, amount);
				break;
			case DESCRIPTOR_HEAP_TYPE::DSV:
				descriptorHeap = context.CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV, amount);
				break;
			case DESCRIPTOR_HEAP_TYPE::RTV:
				descriptorHeap = context.CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV, amount);
				break;
			case DESCRIPTOR_HEAP_TYPE::SAMPLER:
				descriptorHeap = context.CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, amount);
				break;
			default:
				Themp::Print("Attempted to create descriptor heap of undefined type!");
				descriptorHeap = nullptr;
				Themp::Break();
				break;
			}
			tracker.heap = descriptorHeap;
			tracker.usedSlots = 0;
			tracker.maxSlots = amount;
			return descriptorHeap;
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