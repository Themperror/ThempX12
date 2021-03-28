#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace Themp
{
	namespace D3D
	{
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


		extern D3D12_DESCRIPTOR_HEAP_TYPE GetDescriptorHeapType(DESCRIPTOR_HEAP_TYPE type);
	}
}