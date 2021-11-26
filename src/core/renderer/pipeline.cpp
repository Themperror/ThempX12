#include "pipeline.h"
#include "control.h"
#include "device.h"
#include "engine.h"
namespace Themp
{
	namespace D3D
	{
		void Pipeline::Init(const Pass& pass)
		{
			ID3D12PipelineState* pipelineState;
			D3D12_GRAPHICS_PIPELINE_STATE_DESC desc {};
			//desc.
			//Themp::Engine::instance->m_Renderer->GetDevice()->CreateGraphicsPipelineState(&desc, );
		}
	}
}