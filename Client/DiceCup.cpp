#include "ClientPch.h"
#include "DiceCup.h"

#include "RigidBodyComponent.h"
#include "Transform.h"
#include "AnimatorComponent.h"
#include "GameObject.h"

#include "AudioManager.h"

using namespace DirectX;

void DiceCup::Idle()
{
	if (m_rigidBody)
		m_rigidBody->SetColliderEnabled(true, 9);

	if (m_animator)
		m_animator->Play("IDLE");
}

void DiceCup::Shake()
{
	if (m_animator)
		m_animator->Play("SHAKE");
}

void DiceCup::Flip()
{
	m_pendingReturn = false;
	m_returnTimer   = 0.f;

	if (m_animator)
		m_animator->Transition("FLIP", 0.2f);
}

void DiceCup::Return()
{
	if (m_animator)
		m_animator->Play("RETURN");
}

bool DiceCup::IsFlipFinished()
{
	if (m_animator && "FLIP" == m_animator->GetCurrentName() && m_animator->IsFinished())
		return true;

	return false;
}

float3 DiceCup::GetPosition()
{
	return m_transform ? m_transform->GetPosition() : float3(0.f, 0.f, 0.f);
}


void DiceCup::Start()
{
	m_transform = GetOwner()->GetComponent<GameEngine::Transform>();
	m_rigidBody = GetOwner()->GetComponent<GameEngine::RigidBodyComponent>();
	m_animator = GetOwner()->GetComponent<GameEngine::AnimatorComponent>();
}

void DiceCup::Update(float dt)
{
	if (IsFlipFinished())
	{
		if (m_isOwnerTurn)
			m_rigidBody->SetColliderEnabled(false, 9);

		if (!m_pendingReturn)
		{
			m_pendingReturn = true;
			m_returnTimer   = 0.f;
		}
	}

	if (m_pendingReturn)
	{
		m_returnTimer += dt;
		if (m_returnTimer >= RETURN_DELAY)
		{
			m_pendingReturn = false;
			Return();
		}
	}
}

void DiceCup::OnCollisionEnter(GameEngine::RigidBodyComponent* other)
{
	RigidBody* body = m_rigidBody->GetRigidBody();
	RigidBody* otherBody = other ? other->GetRigidBody() : nullptr;

	Vec3 velA = body->GetLinearVelocity();
	Vec3 velB = otherBody ? otherBody->GetLinearVelocity() : Vec3{ 0,0,0 };
	float relSpeed = Vec3Length(Vec3{ velA.x - velB.x, velA.y - velB.y, velA.z - velB.z });

	if (3.f > relSpeed)
		return;

	float volume = std::clamp(relSpeed / 15.f, 0.05f, 1.f);

	int idx = RandInt(1, 2);
	std::string key = "Roller" + std::to_string(idx);

	GameEngine::GAudioManager->PlaySFX(key, volume);
}

