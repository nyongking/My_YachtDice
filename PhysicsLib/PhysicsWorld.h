#pragma once
#include <vector>
#include "IJobDispatcher.h"
#include "Contact.h"

#include "BroadPhase.h"
#include "NarrowPhase.h"
#include "ContactSolver.h"
#include "IslandBuilder.h"
#include "Ray.h"


class RigidBody;
struct Collider;
struct IJobDispatcher;

class PhysicsWorld
{
public:
	void	Step(float dt);

	void	AddRigidBody(RigidBody* body);
	void	RemoveRigidBody(RigidBody* body);
	void	ClearRigidBody();

	void	AddCollider(Collider* collider);
	void	RemoveCollider(Collider* collider);
	void	ClearColliders();

	void	SetDispatcher(IJobDispatcher* dispatcher);

	const std::vector<Contact>& GetCollisions() const { return m_collisions; }
	const std::vector<Contact>& GetTriggers() const { return m_triggers; }

	void SetGravity(const Vec3& gravity) { m_gravity = gravity; }
	Vec3 GetGravity() const { return m_gravity; }

	// ── Raycast ──
	bool Raycast(const Ray& ray, RayHit& outHit) const;
	int  RaycastAll(const Ray& ray, std::vector<RayHit>& outHits) const;


private:
	std::vector<RigidBody*> m_bodies;
	std::vector<Collider*>	m_colliders;

	std::vector<ColliderPair> m_pairs;
	std::vector<Contact>	  m_collisions;
	std::vector<Contact>	 m_triggers;

	BroadPhase				m_broadPhase;
	NarrowPhase				m_narrowPhase;
	IslandBuilder			m_islandBuilder;


	IJobDispatcher*			m_dispatcher = nullptr;
	SingleThreadDispatcher	m_defaultDispatcher;
	Vec3					m_gravity = Vec3(0.f, -20.f, 0.f);
};
