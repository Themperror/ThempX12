#pragma once

#include "context.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using namespace Microsoft::WRL;

namespace Themp
{
	namespace D3D
	{
		class Texture;
		class Frame
		{
		public:
			void Init(int bufferIndex, RTVHeap heap);
			void Reset();
			void Present();
			ComPtr<ID3D12CommandAllocator> GetCommandAllocator() const;
			const Texture& GetFrameBuffer() const;
			ComPtr<ID3D12GraphicsCommandList> GetCmdList() const;
		private:
			ComPtr<ID3D12GraphicsCommandList> m_CommandList;
			ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
			const Texture* m_FrameBuffer;

			size_t m_rtvHeapStart;
			int m_rtvHeapIndex;
			int m_rtvDescriptorSize;
		};
	}
}