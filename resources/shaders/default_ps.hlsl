#include "default_root.h"

struct PSOut
{
	float4 color : SV_Target0;
	float depth : SV_Depth;
};

[RootSignature(ROOT)]
PSOut main(float4 pos : SV_Position)
{
	PSOut psOut;
	psOut.color = float4(pos.xyz,0.33);
	psOut.color.x /= 800.0;
	psOut.color.y /= 600.0;
	psOut.depth = psOut.color.x;
	return psOut;
}