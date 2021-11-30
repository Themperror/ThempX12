#include "shadow_root.h"
[RootSignature(ROOT)]
float main(float4 pos : SV_POSITION) : SV_Depth
{
	return pos.y / 600.0f;
}