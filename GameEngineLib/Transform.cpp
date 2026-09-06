#include "GameEnginePch.h"
#include "Transform.h"

namespace GameEngine
{
	void Transform::SetPosition(const float3& pos)
	{
		m_position = pos;
		m_dirty    = true;
	}

	void Transform::SetRotation(const float3& eulerDeg)
	{
		m_rotation = eulerDeg;
		m_dirty    = true;
	}

	void Transform::SetScale(const float3& scale)
	{
		m_scale = scale;
		m_dirty = true;
	}

	MyJson Transform::Serialize() const
	{
		MyJson j;
		j["position"] = { m_position.x, m_position.y, m_position.z };
		j["rotation"] = { m_rotation.x, m_rotation.y, m_rotation.z };
		j["scale"]    = { m_scale.x,    m_scale.y,    m_scale.z    };
		return j;
	}

	void Transform::Deserialize(const MyJson& j)
	{
		if (j.contains("position"))
		{
			m_position = { j["position"][0], j["position"][1], j["position"][2] };
			m_dirty = true;
		}
		if (j.contains("rotation"))
		{
			m_rotation = { j["rotation"][0], j["rotation"][1], j["rotation"][2] };
			m_dirty = true;
		}
		if (j.contains("scale"))
		{
			m_scale = { j["scale"][0], j["scale"][1], j["scale"][2] };
			m_dirty = true;
		}
	}

	float4x4 Transform::GetWorldMatrix() const
	{
		if (!m_dirty)
			return m_worldMatrix;

		using namespace DirectX;

		const float toRad = XM_PI / 180.f;

		XMMATRIX S = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
		// ImGuizmo DecomposeMatrixToComponents uses Rx * Ry * Rz (XYZ) order
		XMMATRIX R = XMMatrixRotationX(m_rotation.x * toRad)
		           * XMMatrixRotationY(m_rotation.y * toRad)
		           * XMMatrixRotationZ(m_rotation.z * toRad);
		XMMATRIX T = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);

		XMStoreFloat4x4(&m_worldMatrix, S * R * T);
		m_dirty = false;

		return m_worldMatrix;
	}
}
