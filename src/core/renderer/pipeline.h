#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

using namespace Microsoft::WRL;

namespace Themp
{
	namespace D3D
	{
		struct SubPass;
		class Pipeline
		{
		public:
			void Init(const SubPass& pass);
			ComPtr<ID3D12PipelineState> m_Pipeline;
		};
	}
}