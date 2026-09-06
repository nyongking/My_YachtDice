#pragma once
#include <Component.h>
#include "Vec3.h"
#include "Quat.h"

struct DiceTransformData;

namespace GameEngine
{
	class RigidBodyComponent;
	class Transform;
}

class Dice : public GameEngine::Component
{
public:
	enum class State
	{
		IDLE,
		MOVE,
		THROWN,
		NETWORK_DRIVEN,
	};

public:
	bool CheckState(State state) { return m_state == state; }

	void PoseInCup(int value, const float3& position);
	void Impulse(const float3& impulse, const float3& point);
	void PoseAtCamera(const float3& position);
	void PoseAtCutter(const float3& pos);

	// Network-driven mode
	void SetNetworkDriven(bool enabled);
	void SetNetworkTransform(const Vec3& pos, const Quat& rot);
	void ApplyServerSettle(int topFace, const Vec3& pos, const Quat& rot);

	int GetTopFace() const { return m_topFace; }
	int GetSlot() const { return m_slot; }
	void SetSlot(int slot) { m_slot = slot; }

public:
	void Start() override;
	void Update(float dt) override;
	void OnTriggerEnter(GameEngine::RigidBodyComponent* other) override;
	void OnCollisionEnter(GameEngine::RigidBodyComponent* other) override;

private:
	static bool IsValidFace(int value) { return value >= 1 && value <= 6; }

	void SetState(State state) { m_state = state; }
	void DetermineTopFace();
	void LerpMoveTo(float dt);

	// Physics helpers
	void ZeroVelocity();
	void EnableLayerBit(unsigned int bit);
	void DisableLayerBit(unsigned int bit);

private:
	 GameEngine::RigidBodyComponent* m_rigidBody = nullptr;
	 GameEngine::Transform*			 m_transform = nullptr;

	 State							 m_state = State::IDLE;
	 int							 m_topFace = 0;
	 int							 m_slot = -1;

	 // Lerp movement
	 float							 m_lerpProgress = 0.f;
	 float3							 m_startPos = {};
	 float3							 m_startRot = {};
	 float3							 m_destPos = {};

	 // Network interpolation
	 Vec3  m_netPrevPos = {};
	 Quat  m_netPrevRot = { 0,0,0,1 };
	 Vec3  m_netTargetPos = {};
	 Quat  m_netTargetRot = { 0,0,0,1 };
	 float m_netLerpT = 1.f;
	 static constexpr float NET_SNAPSHOT_INTERVAL = 1.f / 30.f;  // 30Hz
};
