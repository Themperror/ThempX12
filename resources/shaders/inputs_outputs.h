#pragma pack_matrix( row_major )
struct EngineConstants
{
	float time;
	float screenWidth;
	float screenHeight;
	float d0;
};

struct CameraConstants
{
	matrix viewMatrix;
	matrix projectionMatrix;
	matrix invProjectionMatrix;
	matrix invViewMatrix;
	matrix viewProjMatrix;
	float4 cameraPosition;
	float4 cameraDir;
};

struct InstanceVSInput
{
	matrix modelMatrix : PER_INSTANCE_MATRIX;
	uint instanceID: SV_InstanceID;
};

struct PositionVSInput
{
	float3 pos : POSITION;
};

struct NormalVSInput
{
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

struct UVVSInput
{
	float2 uv : UV;
};
