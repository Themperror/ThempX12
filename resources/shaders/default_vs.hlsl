#include "default_root.h"

[RootSignature(ROOT)]
VSOutput main(PositionVSInput posData, InstanceVSInput instanceData, NormalVSInput normalData, UVVSInput uvData)
{
	VSOutput output;
	matrix worldMatrix = mul(instanceData.modelMatrix, cam.viewProjMatrix);
	output.pos = mul(float4(posData.pos,1.0), worldMatrix);
	output.uv = uvData.uv;
	output.normal =  normalData.normal.xyzz;
	output.instanceID = instanceData.instanceID;
	return output;
}