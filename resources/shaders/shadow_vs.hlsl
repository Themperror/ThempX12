#include "shadow_root.h"
#include "inputs_outputs.h"
[RootSignature(ROOT)]
DefaultPSInput main(DefaultVSInput input)
{
	DefaultPSInput output;
	output.pos = float4(input.pos.xy + float2(input.modelMatrix._m03, input.modelMatrix._m13), 0.5, 1.0);
	output.instanceID = input.instanceID;
	return output;
}