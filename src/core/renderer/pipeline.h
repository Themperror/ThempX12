#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <array>
#include "types.h"

using namespace Microsoft::WRL;

namespace Themp
{
	namespace D3D
	{
		struct SubPass;
		class Frame;
		class Pipeline
		{
		public:
			void Init(const SubPass& subpass);
			ComPtr<ID3D12PipelineState> m_Pipeline;
			ComPtr<ID3D12RootSignature> m_RootSignature;
			std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_RenderTargets;
			D3D12_CPU_DESCRIPTOR_HANDLE m_DepthTarget{};
			std::array<D3D12_VIEWPORT, 8> m_Viewports;
			std::array<D3D12_RECT, 8> m_Scissors;

			void SetTo(Frame& cmdList);

			PassHandle GetPassHandle() const { return m_PassHandle; }
		private:
			PassHandle m_PassHandle = PassHandle::Invalid;
		};
	}
}