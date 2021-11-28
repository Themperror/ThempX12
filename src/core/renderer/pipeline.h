#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "types.h"
using namespace Microsoft::WRL;

namespace Themp
{
	namespace D3D
	{
		class Pass;
		class Pipeline
		{
		public:
			void Init(const Pass& pass);
			ComPtr<ID3D12PipelineState> m_Pipeline;
		};
	}
}