#pragma once


#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include "types.h"
#include "texture.h"
using namespace Microsoft::WRL;

namespace Themp
{
	namespace D3D
	{
		class Device;
		class GPU_Resources
		{
		public:
			GPU_Resources(const D3D::Device& device, uint32_t maxSRVs, uint32_t maxDSVs, uint32_t maxRTVs, uint32_t maxSamplers);
		public:
			ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap(DESCRIPTOR_HEAP_TYPE type, uint32_t reservedSlots = 0);
			Texture& GetTextureFromResource(ComPtr<ID3D12Device2> device,  ComPtr<ID3D12Resource> resource, TEXTURE_TYPE type);
		private:
			

			DescriptorHeapTracker CreateDescriptorHeap(const D3D::Device& device, DESCRIPTOR_HEAP_TYPE type, uint32_t amount);

			//Texture& MakeTextureFromResource(const Context& device, DescriptorHeapTracker& heapTracker, ComPtr<ID3D12Resource> resource, TEXTURE_TYPE type);
			std::unordered_map<std::string, Texture*> m_FileTextures;
			std::vector< std::unique_ptr<Texture>> m_Textures;

			DescriptorHeapTracker m_CB_SRV_UAV_Heap;
			DescriptorHeapTracker m_RTV_Heap;
			DescriptorHeapTracker m_DSV_Heap;
			DescriptorHeapTracker m_SAMPLER_Heap;
		};
	}
}