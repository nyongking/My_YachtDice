#pragma once
#include <Component.h>

namespace GameEngine
{
	class RigidBodyComponent;
	class Transform;
	class AnimatorComponent;
}

class DiceCup : public GameEngine::Component
{
public:
	void Idle();
	void Shake();
	void Flip();
	void Return();

	bool IsFlipFinished();
	float3 GetPosition();

	void SetOwnerTurn(bool isOwnerTurn) { m_isOwnerTurn = isOwnerTurn; }

public:
	void Start() override;
	void Update(float dt) override;
	void OnCollisionEnter(GameEngine::RigidBodyComponent* other) override;
	 

private:
	GameEngine::RigidBodyComponent* m_rigidBody = nullptr;
	GameEngine::Transform*			m_transform = nullptr;
	GameEngine::AnimatorComponent* m_animator = nullptr;

	bool  m_isOwnerTurn   = false;
	bool  m_pendingReturn = false;
	float m_returnTimer   = 0.f;
	static constexpr float RETURN_DELAY = 1.0f;
};

