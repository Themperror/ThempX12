#include "shadow_root.h"

[RootSignature(ROOT)]
float main(VSOut input) : SV_Depth
{
	return input.pos.z;
}