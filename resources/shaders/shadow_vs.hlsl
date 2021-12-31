#include "shadow_root.h"

[RootSignature(ROOT)]
VSOut main(PositionVSInput posData, InstanceVSInput instanceData)
{
	VSOut output;
	output.pos = mul(float4(posData.pos,1.0), mul(instanceData.modelMatrix, cam.viewProjMatrix));
	output.instanceID = instanceData.instanceID;
	return output;
}