#include "default_root.h"

#include "inputs_outputs.h"

struct PSOut
{
	float4 color : SV_Target0;
	float depth : SV_Depth;
};

[RootSignature(ROOT)]
PSOut main(DefaultPSInput input)
{
	PSOut psOut;
	psOut.color = float4(input.pos.xy, 0 ,0.33);
	psOut.color.y = input.instanceID;
	psOut.color.y /= 4.0;
	psOut.color.z = input.instanceID;
	psOut.color.z /= 4.0;
	psOut.color.x = input.instanceID;
	psOut.color.x /= 4.0;
	psOut.depth = 0.5;
	return psOut;
}