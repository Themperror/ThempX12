#include "shadow_root.h"
[RootSignature(ROOT)]
float4 main(float3 pos : POSITION) : SV_POSITION
{
	return float4(pos.xyz, 1.0);
}