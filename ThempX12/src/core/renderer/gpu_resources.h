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
		class Context;
		class GPU_Resources
		{
		public:

		public:
			DescriptorHeapTracker& GetDescriptorHeap(const D3D::Context& context, DESCRIPTOR_HEAP_TYPE type, uint32_t amount);
			Texture& GetTextureFromResource(const Context& device, DescriptorHeapTracker& heapTracker, ComPtr<ID3D12Resource> resource, TEXTURE_TYPE type);
		private:
			
			DescriptorHeapTracker& MakeDescriptorHeapTracker(DESCRIPTOR_HEAP_TYPE type);

			ComPtr<ID3D12DescriptorHeap> MakeDescriptorHeap(const D3D::Context& context, DescriptorHeapTracker& tracker, DESCRIPTOR_HEAP_TYPE type, uint32_t amount);

			//Texture& MakeTextureFromResource(const Context& device, DescriptorHeapTracker& heapTracker, ComPtr<ID3D12Resource> resource, TEXTURE_TYPE type);
			std::unordered_map<std::string, Texture*> m_FileTextures;
			std::vector< std::unique_ptr<Texture>> m_Textures;

			std::vector<std::unique_ptr<DescriptorHeapTracker>> m_CB_SRV_UAV_Heaps;
			std::vector<std::unique_ptr<DescriptorHeapTracker>> m_RTV_Heaps;
			std::vector<std::unique_ptr<DescriptorHeapTracker>> m_DSV_Heaps;
			std::vector<std::unique_ptr<DescriptorHeapTracker>> m_SAMPLER_Heaps;
		};
	}
}