#include "core/components/transform.h"


namespace Themp
{
	Transform::Transform(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, DirectX::XMFLOAT3 scale)
	{
		m_Position = position;
		m_Rotation = rotation;
		m_Scale = scale;
	}

	void Transform::SetPosition(DirectX::XMFLOAT3 position)
	{
		m_Dirty = true;
		m_Position = position;
	}
	
	void Transform::SetPosition(float x, float y, float z)
	{
		m_Dirty = true;
		m_Position = DirectX::XMFLOAT3(x, y, z);
	}

	void Transform::SetRotation(DirectX::XMFLOAT3 rotation)
	{
		m_Dirty = true;
		m_Rotation = rotation;
	}
	void Transform::SetRotation(float x, float y, float z)
	{
		m_Dirty = true;
		m_Rotation = DirectX::XMFLOAT3(x, y, z);
	}

	void Transform::SetScale(DirectX::XMFLOAT3 scale)
	{
		m_Dirty = true;
		m_Scale = scale;
	}
	void Transform::SetScale(float x, float y, float z)
	{
		m_Dirty = true;
		m_Scale = DirectX::XMFLOAT3(x, y, z);
	}

	void Transform::AddPosition(DirectX::XMFLOAT3 position)
	{
		m_Dirty = true;
		m_Position.x += position.x;
		m_Position.y += position.y;
		m_Position.z += position.z;
	}

	void Transform::AddPosition(float x, float y, float z)
	{
		m_Dirty = true;
		m_Position.x += x;
		m_Position.y += y;
		m_Position.z += z;
	}

	DirectX::XMFLOAT4X4 Transform::GetModelMatrix()
	{
		using namespace DirectX;
		if (m_Dirty)
		{
			m_Dirty = false;
			XMMATRIX translation = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
			XMMATRIX rotation = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
			XMMATRIX scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);

			XMMATRIX model = XMMatrixMultiply(rotation, scale);
			model = XMMatrixMultiply(model, translation);

			XMStoreFloat4x4(&m_ModelMatrix, model);
		}
		return m_ModelMatrix;
	}
}