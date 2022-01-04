#include "default_root.h"

struct PSOut
{
	float4 color : SV_Target0;
	float depth : SV_Depth;
};


float3 Hue(float H)
{
	float R = abs(H * 6 - 3) - 1;
	float G = 2 - abs(H * 6 - 2);
	float B = 2 - abs(H * 6 - 4);
	return saturate(float3(R,G,B));
}

float4 HSVtoRGB(in float3 HSV)
{
	return float4(((Hue(HSV.x) - 1) * HSV.y + 1) * HSV.z,1);
}


[RootSignature(ROOT)]
PSOut main(VSOutput input)
{
	PSOut psOut;
	float col = fmod(engine.time + input.instanceID / 4.0, 1.0);
	
	float3 lightPos = float3(0,0,0);
	float4 worldFragmentPos = mul(input.pos, mul(cam.invViewMatrix, cam.invProjectionMatrix));
	
    float dist = distance(worldFragmentPos.xyz, lightPos.xyz);
	
	float lightStrength = 1.0 / ((dist * dist) / brightness);
	
	float lightNormal = saturate(dot(input.normal.xyz, lightPos));
	
	psOut.color = diffuse.Sample(samplerState, input.uv);
	psOut.depth = input.pos.z;
	return psOut;
}