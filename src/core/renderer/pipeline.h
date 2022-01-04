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
		struct RenderPass;
		class Frame;

		struct DescriptorTableInfo
		{
			int slot;
			int numTexturesExpected;
		};
		class Pipeline
		{
		public:
			void Init(const SubPass& subpass);
			ComPtr<ID3D12PipelineState> m_Pipeline;
			ComPtr<ID3D12RootSignature> m_RootSignature;
			std::vector<std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_CLEAR_VALUE>> m_RenderTargets;
			std::pair < D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_CLEAR_VALUE> m_DepthTarget{};
			std::array<D3D12_VIEWPORT, 8> m_Viewports;
			std::array<D3D12_RECT, 8> m_Scissors;

			void SetTo(Frame& frame, RenderPass& renderPass);
			void ClearTargets(Frame& frame);

			PassHandle GetPassHandle() const { return m_PassHandle; }
			const std::vector<DescriptorTableInfo>& GetDescriptorTables() const { return m_DescriptorTables; }
		private:
			PassHandle m_PassHandle = PassHandle::Invalid;
			std::vector<DescriptorTableInfo> m_DescriptorTables;
		};
	}
}