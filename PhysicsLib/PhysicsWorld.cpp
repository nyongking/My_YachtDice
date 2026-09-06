#include "PhysicsPch.h"
#include "PhysicsWorld.h"

#include "RigidBody.h"
#include "Collider.h"
#include "RayCast.h"
#include <algorithm>

void PhysicsWorld::Step(float dt)
{
	auto* dp = m_dispatcher != nullptr ? m_dispatcher : &m_defaultDispatcher;

	// step1 : 중력 적용 + 속도 적분 (sleeping 바디는 내부 guard로 건너뜀)
	dp->ParallelFor(0, static_cast<int>(m_bodies.size()), [&](int i)
		{
			RigidBody* body = m_bodies[i];

			if (body->IsSleeping())
				return;

			if (body->IsDynamic())
			{
				body->ApplyForce(m_gravity * (1.f / body->GetInvMass()));
				body->ApplyOrientationInjection();
			}

			body->IntegrateVelocity(dt);
			//body->UpdateWorldInertia();
		});

	m_pairs.clear();
	m_collisions.clear();
	m_triggers.clear();

	// step2 : 충돌 감지 (sleeping 바디 포함 — 새 충돌 감지 위해)
	m_broadPhase.GetPairsBruteForce(m_colliders, m_pairs);

	for (auto& pair : m_pairs)
		m_narrowPhase.TestPair(pair, m_collisions, m_triggers);

	// step2.5 : 충돌 기반 Wake — awake 바디와 충돌한 sleeping 바디 깨움
	for (auto& ct : m_collisions)
	{
		RigidBody* a = ct.bodyA;
		RigidBody* b = ct.bodyB;
		if (!a || !b) continue;

		bool aAwakeDynamic = a->IsDynamic() && !a->IsSleeping();
		bool bAwakeDynamic = b->IsDynamic() && !b->IsSleeping();

		if (a->IsSleeping() && (bAwakeDynamic || b->IsKinematic()))
			a->WakeUp();
		if (b->IsSleeping() && (aAwakeDynamic || a->IsKinematic()))
			b->WakeUp();
	}

	// step3 : pseudo velocity 초기화 + 충돌 해결 (Split Impulse)
	dp->ParallelFor(0, static_cast<int>(m_bodies.size()), [&](int i)
		{
			m_bodies[i]->ClearPseudoVelocities();
		});

	if (!m_collisions.empty())
	{
		std::vector<RigidBody*> bodyTemps;
		bodyTemps.reserve(m_bodies.size());

		for (auto& body : m_bodies)
			bodyTemps.push_back(body);

		m_islandBuilder.Build(bodyTemps, m_collisions);

		const std::vector<Island>& islands = m_islandBuilder.GetIslands();

		// Island 기반 Wake: 한 Island 내에 awake 바디가 있으면 전체 깨움
		for (const auto& island : islands)
		{
			bool hasAwake = false;
			for (auto* body : island.bodies)
			{
				if (body->IsDynamic() && !body->IsSleeping())
				{
					hasAwake = true;
					break;
				}
			}
			if (hasAwake)
			{
				for (auto* body : island.bodies)
				{
					if (body->IsSleeping())
						body->WakeUp();
				}
			}
		}

		dp->ParallelFor(0, static_cast<int>(islands.size()), [&](int i)
			{
				const Island& land = islands[i];
				if (land.contacts.empty())
					return;

				// Island 내 모든 Dynamic 바디가 sleeping이면 솔빙 건너뜀
				bool allSleeping = true;
				for (auto* body : land.bodies)
				{
					if (body->IsDynamic() && !body->IsSleeping())
					{
						allSleeping = false;
						break;
					}
				}
				if (allSleeping)
					return;

				std::vector<Contact> islandContacts;
				islandContacts.reserve(land.contacts.size());

				for (auto* ct : land.contacts)
					islandContacts.push_back(*ct);

				ContactSolver solver;
				solver.Resolve(islandContacts, dt);
			});
	}

	// step4 : 위치/회전 갱신 (sleeping 바디는 내부 guard로 건너뜀)
	dp->ParallelFor(0, static_cast<int>(m_bodies.size()), [&](int i)
		{
			m_bodies[i]->IntegratePosition(dt);
		});

	// step5 : 클리어
	dp->ParallelFor(0, (int)m_bodies.size(), [&](int i)
		{
			m_bodies[i]->ClearAccumulators();
		});

	// step6 : Sleep 상태 갱신
	dp->ParallelFor(0, static_cast<int>(m_bodies.size()), [&](int i)
		{
			m_bodies[i]->UpdateSleepState(dt);
		});

}

void PhysicsWorld::AddRigidBody(RigidBody* body)
{
	m_bodies.push_back(body);
}

void PhysicsWorld::RemoveRigidBody(RigidBody* body)
{
	for (auto it = m_bodies.begin(); it != m_bodies.end(); ++it)
	{
		if (*it == body)
		{
			m_bodies.erase(it);
			return;
		}
	}
}

void PhysicsWorld::ClearRigidBody()
{
	m_bodies.clear();
}

void PhysicsWorld::AddCollider(Collider* collider)
{
	m_colliders.push_back(collider);
}

void PhysicsWorld::RemoveCollider(Collider* collider)
{
	for (auto it = m_colliders.begin(); it != m_colliders.end(); ++it)
	{
		if (*it == collider)
		{
			m_colliders.erase(it);
			return;
		}
	}
}

void PhysicsWorld::ClearColliders()
{
	m_colliders.clear();
}

void PhysicsWorld::SetDispatcher(IJobDispatcher* dispatcher)
{
	m_dispatcher = dispatcher;
}

// ── Raycast ──

bool PhysicsWorld::Raycast(const Ray& ray, RayHit& outHit) const
{
	float closestT = FLT_MAX;
	Collider* closestCol = nullptr;

	for (auto* col : m_colliders)
	{
		if (!col->enabled)
			continue;

		// 레이어 필터링
		if (!(ray.layerMask & (1u << col->layer)))
			continue;

		// 1차: AABB 필터
		AABB aabb = col->GetAABB();
		float tAABB;
		if (!RayIntersectAABB(ray, aabb, tAABB))
			continue;
		if (tAABB >= closestT)
			continue;

		// 2차: 정밀 테스트
		float t;
		bool hit = false;

		if (col->type == Collider::ColliderType::Sphere)
		{
			auto* sc = static_cast<SphereCollider*>(col);
			hit = RayIntersectSphere(ray, col->GetWorldPosition(), sc->radius, t);
		}
		else if (col->type == Collider::ColliderType::Box)
		{
			auto* bc = static_cast<BoxCollider*>(col);
			hit = RayIntersectOBB(ray, col->GetWorldPosition(),
				bc->halfExtents, col->GetWorldOrientation(), t);
		}

		if (hit && t < closestT)
		{
			closestT = t;
			closestCol = col;
		}
	}

	if (!closestCol)
		return false;

	outHit.distance  = closestT;
	outHit.point     = ray.origin + ray.direction * closestT;
	outHit.collider  = closestCol;
	return true;
}

int PhysicsWorld::RaycastAll(const Ray& ray, std::vector<RayHit>& outHits) const
{
	outHits.clear();

	for (auto* col : m_colliders)
	{
		if (!col->enabled)
			continue;

		// 레이어 필터링
		if (!(ray.layerMask & (1u << col->layer)))
			continue;

		AABB aabb = col->GetAABB();
		float tAABB;
		if (!RayIntersectAABB(ray, aabb, tAABB))
			continue;

		float t;
		bool hit = false;

		if (col->type == Collider::ColliderType::Sphere)
		{
			auto* sc = static_cast<SphereCollider*>(col);
			hit = RayIntersectSphere(ray, col->GetWorldPosition(), sc->radius, t);
		}
		else if (col->type == Collider::ColliderType::Box)
		{
			auto* bc = static_cast<BoxCollider*>(col);
			hit = RayIntersectOBB(ray, col->GetWorldPosition(),
				bc->halfExtents, col->GetWorldOrientation(), t);
		}

		if (hit)
		{
			RayHit rh;
			rh.distance = t;
			rh.point    = ray.origin + ray.direction * t;
			rh.collider = col;
			outHits.push_back(rh);
		}
	}

	// 거리순 정렬
	std::sort(outHits.begin(), outHits.end(),
		[](const RayHit& a, const RayHit& b) { return a.distance < b.distance; });

	return static_cast<int>(outHits.size());
}
