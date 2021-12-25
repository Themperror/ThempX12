#pragma once

#include <DirectXMath.h>

namespace Themp
{
	class Transform
	{
	public:
		Transform(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, DirectX::XMFLOAT3 scale);
		void SetPosition(DirectX::XMFLOAT3 position);
		void SetPosition(float x, float y, float z);
		void SetRotation(DirectX::XMFLOAT3 rotation);
		void SetRotation(float x, float y, float z);
		void SetScale(DirectX::XMFLOAT3 scale);
		void SetScale(float x, float y, float z);
		void AddPosition(DirectX::XMFLOAT3 position);
		void AddPosition(float x, float y, float z);
		DirectX::XMFLOAT4X4 GetModelMatrix();

	private:
		bool m_Dirty = true;
		DirectX::XMFLOAT3 m_Position;
		DirectX::XMFLOAT3 m_Rotation;
		DirectX::XMFLOAT3 m_Scale;

		DirectX::XMFLOAT4X4 m_ModelMatrix;
		DirectX::XMFLOAT4X4 m_WorldMatrix;
	};

}