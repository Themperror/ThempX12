struct DefaultVSInput
{
	matrix modelMatrix : PER_INSTANCE_MATRIX;
	float3 pos : POSITION;
	uint instanceID: SV_InstanceID;
};

struct DefaultPSInput
{
	float4 pos : SV_Position;
	uint instanceID : SV_InstanceID;
};