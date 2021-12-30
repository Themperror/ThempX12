void main()
{
	Print("Doing rendering logic for Mainpass!");
	ConstantBufferHandle handle = GetConstantBuffer();
	//SetConstantBuffer(0, CBufferType::Camera);
	
	DeclareConstantBufferMemberFloat(handle, "time");
	DeclareConstantBufferMemberFloat(handle, "delta");
	DeclareConstantBufferMemberFloat2(handle, "uv");
	DeclareConstantBufferMemberFloat2(handle, "uv2");
	
	float2 uv;
	uv.x = 0.5f;
	uv.g = 1.0f;
	
	SetConstantBufferData(handle, "uv", uv);
	SetConstantBufferData(handle, "uv2", uv);
	
	SetConstantBuffer(0, handle);
	
	
	while(true)
	{
		yield();
		SetConstantBufferData(handle, "time", GetTime());
		SetConstantBufferData(handle, "delta", GetDeltaTime());
	}
}
