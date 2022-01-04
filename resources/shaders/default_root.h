#define ROOT \
"RootFlags( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
"CBV(b0, space = 0)," \
"CBV(b1, space = 0)," \
"CBV(b2, space = 0)," \
"DescriptorTable( SRV(t0, numDescriptors=1) ), "\
"DescriptorTable( SRV(t1, numDescriptors=1) ), "\
"StaticSampler(s0, " \
	 "addressU = TEXTURE_ADDRESS_WRAP, " \
	 "addressV = TEXTURE_ADDRESS_WRAP, " \
	 "filter = FILTER_MIN_MAG_MIP_LINEAR )"

#include "inputs_outputs.h"


cbuffer Camera : register(b0)
{
  CameraConstants cam;
};

cbuffer Engine : register(b1)
{
  EngineConstants engine;
};

cbuffer MainPassConstants : register(b2)
{
	float brightness;
	float2 mainUv0;
	float mainD0;
	float2 mainUv1;
	float mainD1;
	float mainD2;
};


Texture2D diffuse : register(t0);
SamplerState samplerState : register(s0);

struct VSOutput
{
	float4 pos : SV_Position;
	float2 uv : UV;
	float4 normal : NORMAL;
	uint instanceID :SV_InstanceID;
};