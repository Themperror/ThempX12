#include "default_root.h"
[RootSignature(ROOT)]
float4 main(float4 pos : SV_POSITION) : SV_TARGET0
{
	return 1.0f.xxxx;
}