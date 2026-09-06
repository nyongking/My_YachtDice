#pragma once

#include "PhysicsWorld.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <set>

namespace GameEngine
{
	class RigidBodyComponent;

	class PhysicsManager
	{
	public:
		bool Initialize();

		void Step(float dt);

		// 씬 전환 시 호출 - 모든 body/collider 제거
		void Clear();

		void SetFixedTimeStep(float fixedDt) { m_fixedDt = fixedDt; }
		PhysicsWorld& GetWorld() { return m_world; }

		// RigidBody -> RigidBodyComponent 매핑
		void RegisterBodyMapping(RigidBody* body, RigidBodyComponent* comp);
		void UnregisterBodyMapping(RigidBody* body);
		RigidBodyComponent* FindComponent(RigidBody* body) const;

		// Raycast
		bool Raycast(const Ray& ray, RayHit& outHit) const;
		int  RaycastAll(const Ray& ray, std::vector<RayHit>& outHits) const;

#ifdef _DEBUG
		// 충돌 속도 모니터링
		void SetWatchedBody(RigidBody* body)  { m_watchedBody = body; }
		void ClearWatchedBody()               { m_watchedBody = nullptr; }
		RigidBody* GetWatchedBody() const     { return m_watchedBody; }
		void DrawConsole();
#endif

	private:
		void ProcessCollisionCallbacks();
		void ProcessTriggerCallbacks();

		PhysicsWorld m_world;

		float m_fixedDt     = 1.f / 60.f;  // 고정 물리 스텝 (60Hz)
		float m_accumulator = 0.f;          // 누적 시간
		float m_maxAccum    = 0.2f;         // 최대 누적 (폭주 방지, ~12스텝)

		//  RigidBody -> RigidBodyComponent 매핑
		std::unordered_map<RigidBody*, RigidBodyComponent*> m_bodyMap;

		// 트리거 추적 (Enter/Stay/Exit 판별)
		using TriggerPair = std::pair<Collider*, Collider*>;
		static TriggerPair MakePair(Collider* a, Collider* b)
		{
			return (a < b) ? TriggerPair{a, b} : TriggerPair{b, a};
		}
		std::set<TriggerPair> m_prevCollisionPairs;
		std::set<TriggerPair> m_prevTriggerPairs;

#ifdef _DEBUG
		// 충돌 모니터링
		RigidBody*               m_watchedBody = nullptr;
		std::vector<std::string> m_collisionLog;
		static const size_t      MAX_LOG_ENTRIES = 500;
		std::string              m_lastSaveMessage;
#endif
	};
}
