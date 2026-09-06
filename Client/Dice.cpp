#include "ClientPch.h"
#include "Dice.h"

#include "RigidBodyComponent.h"
#include "Transform.h"
#include "GameObject.h"
#include "AudioManager.h"
#include "NetworkEvents.h"
#include "PhysicsConvert.h"

namespace
{
	constexpr float		BODY_MASS = 5.f;
	constexpr float		SETTLE_THRESHOLD = 0.4f;
	constexpr float		LERP_SPEED = 4.f;

	constexpr unsigned int DICE_COLLISION_BIT = 4;
	constexpr unsigned int CUTTER_LAYER = 3;

	const	  Vec3		UP_WORLD = { 0.f, 1.f, 0.f };

	constexpr float3 DIRECTIONS[6] =
	{
		{0.f, 0.f, 0.f},     // 1
		{90.f, 0.f, 0.f},    // 2
		{0.f, 0.f, -90.f},   // 3
		{0.f, 0.f, 90.f},    // 4
		{-90.f, 0.f, 0.f},   // 5
		{180.f, 0.f, 0.f}    // 6
	};
}

// --- Physics helpers ---

void Dice::ZeroVelocity()
{
	RigidBody* body = m_rigidBody->GetRigidBody();
	if (body)
	{
		body->SetLinearVelocity(Vec3{ 0.f, 0.f, 0.f });
		body->SetAngularVelocity(Vec3{ 0.f, 0.f, 0.f });
	}
}

void Dice::EnableLayerBit(unsigned int bit)
{
	Collider* col = m_rigidBody->GetCollider();
	m_rigidBody->SetCollisionLayer(col->layer, col->layerMask | (1u << bit));
}

void Dice::DisableLayerBit(unsigned int bit)
{
	Collider* col = m_rigidBody->GetCollider();
	m_rigidBody->SetCollisionLayer(col->layer, col->layerMask & ~(1u << bit));
}

// --- Lifecycle ---

void Dice::Start()
{
	m_rigidBody = GetOwner()->GetComponent<GameEngine::RigidBodyComponent>();
	m_transform = GetOwner()->GetComponent<GameEngine::Transform>();
}

void Dice::Update(float dt)
{
	switch (m_state)
	{
	case State::THROWN:
	{
		if (!m_rigidBody)
			break;

		RigidBody* body = m_rigidBody->GetRigidBody();
		if (body)
		{
			float speed    = Vec3Length(body->GetLinearVelocity());
			float rotSpeed = Vec3Length(body->GetAngularVelocity());

			if (speed < SETTLE_THRESHOLD && rotSpeed < SETTLE_THRESHOLD)
			{
				SetState(State::IDLE);
				DetermineTopFace();
			}
		}
		break;
	}
	case State::MOVE:
	{
		LerpMoveTo(dt);
		break;
	}
	case State::NETWORK_DRIVEN:
	{
		if (!m_transform) break;

		m_netLerpT += dt / NET_SNAPSHOT_INTERVAL;
		if (m_netLerpT > 1.f) m_netLerpT = 1.f;

		Vec3 pos = Vec3Lerp(m_netPrevPos, m_netTargetPos, m_netLerpT);
		Quat rot = QuatSlerp(m_netPrevRot, m_netTargetRot, m_netLerpT);

		m_transform->SetPosition(GameEngine::ToEngine(pos));
		m_transform->SetRotation(GameEngine::PhysicsQuatToEuler(rot));
		break;
	}

	default:
		break;
	}
}


void Dice::OnTriggerEnter(GameEngine::RigidBodyComponent* other)
{
	if (!other)
		return;

	Collider* otherCol = other->GetCollider();
	if (otherCol && CUTTER_LAYER == otherCol->layer)
	{
		if (m_rigidBody)
			EnableLayerBit(DICE_COLLISION_BIT);

		if (!CheckState(State::MOVE))
			SetState(State::THROWN);
	}
}

void Dice::OnCollisionEnter(GameEngine::RigidBodyComponent* other)
{
	RigidBody* body = m_rigidBody->GetRigidBody();
	RigidBody* otherBody = other ? other->GetRigidBody() : nullptr;

	if (!body || body->IsKinematic() || !otherBody || otherBody->IsKinematic())
		return;

	Vec3 velA = body->GetLinearVelocity();
	Vec3 velB = otherBody ? otherBody->GetLinearVelocity() : Vec3{ 0,0,0 };
	float relSpeed = Vec3Length(Vec3{ velA.x - velB.x, velA.y - velB.y, velA.z - velB.z });

	if (2.f > relSpeed)
		return;

	float volume = std::clamp(relSpeed / 15.f, 0.05f, 1.f);

	int idx = RandInt(1, 6);
	std::string key = "dice_hit" + std::to_string(idx);

	GameEngine::GAudioManager->PlaySFX(key, volume);
}

// --- Public actions ---

void Dice::PoseInCup(int value, const float3& position)
{
	if (!IsValidFace(value) || !m_rigidBody)
		return;

	m_topFace = value;

	m_rigidBody->SetBodyType(RigidBody::BodyType::Dynamic);
	m_rigidBody->SetMass(BODY_MASS);
	m_rigidBody->SetAllCollidersEnabled(true);

	m_rigidBody->SetPosition(position);
	m_rigidBody->SetRotation(DIRECTIONS[m_topFace - 1]);

	DisableLayerBit(DICE_COLLISION_BIT);
	ZeroVelocity();

	m_lerpProgress = 0.f;
	SetState(State::IDLE);
}

void Dice::Impulse(const float3& impulse, const float3& point)
{
	if (m_rigidBody)
		m_rigidBody->ApplyImpulse(impulse, point);
}

void Dice::PoseAtCamera(const float3& position)
{
	if (!m_rigidBody)
		return;

	SetState(State::MOVE);

	m_rigidBody->SetBodyType(RigidBody::BodyType::Kinematic);
	m_rigidBody->SetAllCollidersEnabled(false);

	ZeroVelocity();
	DisableLayerBit(DICE_COLLISION_BIT);

	m_lerpProgress = 0.f;
	m_startPos = m_transform->GetPosition();
	m_startRot = m_transform->GetRotation();
	m_destPos = position;
}

void Dice::PoseAtCutter(const float3& pos)
{
	if (!m_rigidBody)
		return;

	SetState(State::MOVE);

	m_rigidBody->SetBodyType(RigidBody::BodyType::Kinematic);
	m_rigidBody->SetAllCollidersEnabled(false);

	ZeroVelocity();
	DisableLayerBit(DICE_COLLISION_BIT);

	m_lerpProgress = 0.f;
	m_startPos = m_transform->GetPosition();
	m_startRot = m_transform->GetRotation();
	m_destPos = pos;
}

// --- Private impl ---

void Dice::DetermineTopFace()
{
	if (!m_rigidBody || !m_rigidBody->GetRigidBody())
		return;

	Quat q = m_rigidBody->GetRigidBody()->GetOrientation();

	int topFace = 1;
	float maxDot = -FLOAT__MAX;

	for (int i = 0; i < 6; ++i)
	{
		Quat ref = QuatFromEuler(DIRECTIONS[i].x, DIRECTIONS[i].y, DIRECTIONS[i].z);
		Vec3 localNormal = QuatRotateVec3(QuatConjugate(ref), UP_WORLD);
		Vec3 worldNormal = QuatRotateVec3(q, localNormal);

		float dot = Vec3Dot(worldNormal, UP_WORLD);
		if (dot > maxDot)
		{
			maxDot = dot;
			topFace = i + 1;
		}
	}

	m_topFace = topFace;
}

void Dice::LerpMoveTo(float dt)
{
	if (!m_rigidBody)
		return;

	m_lerpProgress += LERP_SPEED * dt;

	if (1.f < m_lerpProgress)
	{
		m_lerpProgress = 1.f;
		m_rigidBody->SetAllCollidersEnabled(true);
		SetState(State::IDLE);
	}

	float3 pos = Lerp(m_startPos, m_destPos, m_lerpProgress);
	m_transform->SetPosition(pos);

	float3 rot = Lerp(m_startRot, DIRECTIONS[m_topFace - 1], m_lerpProgress);
	m_transform->SetRotation(rot);
}

// --- Network-driven mode ---
// enabled: Kinematic 전환, Collider Off
// 
void Dice::SetNetworkDriven(bool enabled)
{
	if (!m_rigidBody)
		return;

	if (enabled)
	{
		SetState(State::NETWORK_DRIVEN);
		m_rigidBody->SetBodyType(RigidBody::BodyType::Kinematic);
		m_rigidBody->SetAllCollidersEnabled(false);
		ZeroVelocity();

		// Initialize interpolation from current position
		if (m_transform)
		{
			m_netPrevPos = GameEngine::ToPhysics(m_transform->GetPosition());
			m_netPrevRot = GameEngine::EulerToPhysicsQuat(m_transform->GetRotation());
			m_netTargetPos = m_netPrevPos;
			m_netTargetRot = m_netPrevRot;
			m_netLerpT = 1.f;
		}
	}
	else
	{
		SetState(State::IDLE);
	}
}

void Dice::SetNetworkTransform(const Vec3& pos, const Quat& rot)
{
	if (m_state != State::NETWORK_DRIVEN)
		return;

	// 이전 속도와 현재 속도 차이 계산 -> 소리 재생에 사용
	Vec3 prevVel = {
		(m_netTargetPos.x - m_netPrevPos.x) / NET_SNAPSHOT_INTERVAL,
		(m_netTargetPos.y - m_netPrevPos.y) / NET_SNAPSHOT_INTERVAL,
		(m_netTargetPos.z - m_netPrevPos.z) / NET_SNAPSHOT_INTERVAL,
	};
	Vec3 newVel = {
		(pos.x - m_netTargetPos.x) / NET_SNAPSHOT_INTERVAL,
		(pos.y - m_netTargetPos.y) / NET_SNAPSHOT_INTERVAL,
		(pos.z - m_netTargetPos.z) / NET_SNAPSHOT_INTERVAL,
	};
	Vec3 deltaVel = { newVel.x - prevVel.x, newVel.y - prevVel.y, newVel.z - prevVel.z };
	float impact = Vec3Length(deltaVel);

	static constexpr float IMPACT_THRESHOLD = 5.f;
	if (impact > IMPACT_THRESHOLD)
	{
		float volume = std::clamp(impact / 30.f, 0.05f, 1.f);
		int idx = RandInt(1, 6);
		std::string key = "dice_hit" + std::to_string(idx);
		GameEngine::GAudioManager->PlaySFX(key, volume);
	}

	// Shift current target to previous, set new target, reset interpolation
	m_netPrevPos = Vec3Lerp(m_netPrevPos, m_netTargetPos, m_netLerpT);
	m_netPrevRot = QuatSlerp(m_netPrevRot, m_netTargetRot, m_netLerpT);
	m_netTargetPos = pos;
	m_netTargetRot = rot;
	m_netLerpT = 0.f;
}

void Dice::ApplyServerSettle(int topFace, const Vec3& pos, const Quat& rot)
{
	if (!m_transform)
		return;

	m_transform->SetPosition(GameEngine::ToEngine(pos));
	m_transform->SetRotation(GameEngine::PhysicsQuatToEuler(rot));

	if (m_rigidBody)
		ZeroVelocity();

	m_topFace = topFace;
	SetState(State::IDLE);
}
