#include "shadow_root.h"
[RootSignature(ROOT)]
float4 main(float3 pos : POSITION) : SV_POSITION
{
	return pos.xyzz;
}