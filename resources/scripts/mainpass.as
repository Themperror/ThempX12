void main()
{
	print("Doing rendering logic for Mainpass!");
	ConstantBufferHandle handle = GetConstantBuffer(4);
	//SetConstantBuffer(0, CBufferType::Camera);
	SetConstantBufferData(handle, 0, 3.141f);
	SetConstantBuffer(0, handle);
}
