#pragma once

#include "Component.h"

namespace GameEngine
{
	class Transform : public Component
	{
	public:
		void SetPosition(const float3& pos);
		void SetRotation(const float3& eulerDeg);
		void SetScale(const float3& scale);

		const float3& GetPosition() const { return m_position; }
		const float3& GetRotation() const { return m_rotation; }
		const float3& GetScale()    const { return m_scale; }

		float4x4 GetWorldMatrix() const;

		// 직렬화 (Component 인터페이스 override)
		std::string GetTypeName()            const override { return "Transform"; }
		MyJson      Serialize()              const override;
		void        Deserialize(const MyJson& j)   override;

	private:
		float3 m_position = { 0.f, 0.f, 0.f };
		float3 m_rotation = { 0.f, 0.f, 0.f };
		float3 m_scale    = { 1.f, 1.f, 1.f };

		mutable bool     m_dirty = true;
		mutable float4x4 m_worldMatrix = {};
	};
}
