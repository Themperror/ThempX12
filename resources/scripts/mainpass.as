void main()
{
	Print("Doing rendering logic for Mainpass!");
	ConstantBufferHandle handle = GetConstantBuffer();
	SetConstantBuffer(0, CBufferType::Camera);
	SetConstantBuffer(1, CBufferType::Engine);
	
	DeclareConstantBufferMemberFloat(handle, "brightness");
	DeclareConstantBufferMemberFloat(handle, "delta");
	DeclareConstantBufferMemberFloat2(handle, "uv");
	DeclareConstantBufferMemberFloat2(handle, "uv2");
	
	float2 uv;
	uv.x = 0.5f;
	uv.g = 1.0f;
	
	SetConstantBufferData(handle, "uv", uv);
	SetConstantBufferData(handle, "uv2", uv);
	
	SetConstantBuffer(2, handle);
	
	
	while(true)
	{
		yield();
		SetConstantBufferData(handle, "brightness", 1.0 / GetTime());
		SetConstantBufferData(handle, "delta", GetDeltaTime());
	}
}
