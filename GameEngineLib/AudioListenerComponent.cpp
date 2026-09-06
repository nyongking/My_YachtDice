#include "GameEnginePch.h"
#include "AudioListenerComponent.h"
#include "AudioManager.h"
#include "GameObject.h"

#ifdef _DEBUG
#include "Imgui/imgui.h"
#endif

namespace GameEngine
{
	void AudioListenerComponent::Start()
	{
		auto* transform = GetOwner()->GetTransform();
		const float3& pos = transform->GetPosition();
		m_lastPosition = pos;

		m_listener.SetPosition(DirectX::XMFLOAT3(pos.x, pos.y, pos.z));
		m_listener.SetOrientation(
			DirectX::XMFLOAT3(0.f, 0.f, 1.f),   // forward (LH)
			DirectX::XMFLOAT3(0.f, 1.f, 0.f));   // up

		if (GAudioManager)
			GAudioManager->SetActiveListener(m_listener);
	}

	void AudioListenerComponent::LateUpdate(float dt)
	{
		auto* transform = GetOwner()->GetTransform();
		const float3& pos = transform->GetPosition();

		if (dt > 0.f)
		{
			// Compute velocity from position delta
			DirectX::XMFLOAT3 vel;
			vel.x = (pos.x - m_lastPosition.x) / dt;
			vel.y = (pos.y - m_lastPosition.y) / dt;
			vel.z = (pos.z - m_lastPosition.z) / dt;
			m_listener.SetVelocity(vel);
		}

		m_listener.SetPosition(DirectX::XMFLOAT3(pos.x, pos.y, pos.z));

		// Derive forward/up from camera inverse view if available,
		// otherwise use a default forward direction
		const float3& rot = transform->GetRotation();
		DirectX::XMVECTOR quat = DirectX::XMQuaternionRotationRollPitchYaw(
			DirectX::XMConvertToRadians(rot.x),
			DirectX::XMConvertToRadians(rot.y),
			DirectX::XMConvertToRadians(rot.z));
		m_listener.SetOrientationFromQuaternion(quat);

		m_lastPosition = pos;

		if (GAudioManager)
			GAudioManager->SetActiveListener(m_listener);
	}

	void AudioListenerComponent::OnDestroy()
	{
		// No cleanup needed; AudioManager holds a copy, not a pointer.
	}

	// ---- Serialization ----

	MyJson AudioListenerComponent::Serialize() const
	{
		MyJson j;
		j["type"] = GetTypeName();
		return j;
	}

	void AudioListenerComponent::Deserialize(const MyJson& /*j*/)
	{
		// No configurable properties
	}

	// ---- Inspector ----

#ifdef _DEBUG
	void AudioListenerComponent::OnInspectorGUI()
	{
		ImGui::Text("Audio Listener (active)");

		const auto& pos = m_listener.Position;
		ImGui::Text("Position: %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);

		const auto& vel = m_listener.Velocity;
		ImGui::Text("Velocity: %.2f, %.2f, %.2f", vel.x, vel.y, vel.z);
	}
#endif
}
