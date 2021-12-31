#define ROOT \
"RootFlags( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
"CBV(b0, space = 0)"

#include "inputs_outputs.h"


cbuffer Camera : register(b0)
{
  CameraConstants cam;
};

struct VSOut
{
	float4 pos : SV_Position;
	uint instanceID : SV_InstanceID;
};