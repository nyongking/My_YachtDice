#include "GameEnginePch.h"
#include "PhysicsManager.h"
#include "RigidBodyComponent.h"
#include "GameObject.h"
#include "Operation.h"
#include "RigidBody.h"

#ifdef _DEBUG
#include "Imgui/imgui.h"
#include <cstdio>
#endif

namespace GameEngine
{
	bool PhysicsManager::Initialize()
	{
		return true;
	}

	void PhysicsManager::RegisterBodyMapping(RigidBody* body, RigidBodyComponent* comp)
	{
		if (body && comp)
			m_bodyMap[body] = comp;
	}

	void PhysicsManager::UnregisterBodyMapping(RigidBody* body)
	{
		m_bodyMap.erase(body);
	}

	RigidBodyComponent* PhysicsManager::FindComponent(RigidBody* body) const
	{
		auto it = m_bodyMap.find(body);
		return (it != m_bodyMap.end()) ? it->second : nullptr;
	}

	void PhysicsManager::ProcessCollisionCallbacks()
	{
		std::set<TriggerPair> currPairs;
		for (auto& ct : m_world.GetCollisions())
		{
			if (!ct.colA || !ct.colB)
				continue;
			currPairs.insert(MakePair(ct.colA, ct.colB));
		}

		for (auto& pair : currPairs)
		{
			auto* compA = FindComponent(pair.first->body);
			auto* compB = FindComponent(pair.second->body);

			if (m_prevCollisionPairs.find(pair) == m_prevCollisionPairs.end())
			{
				// Enter
				if (compA && compA->GetOwner())
					compA->GetOwner()->ForEachComponent([compB](Component* c) { c->OnCollisionEnter(compB); });
				if (compB && compB->GetOwner())
					compB->GetOwner()->ForEachComponent([compA](Component* c) { c->OnCollisionEnter(compA); });
			}
			else
			{
				// Stay
				if (compA && compA->GetOwner())
					compA->GetOwner()->ForEachComponent([compB](Component* c) { c->OnCollisionStay(compB); });
				if (compB && compB->GetOwner())
					compB->GetOwner()->ForEachComponent([compA](Component* c) { c->OnCollisionStay(compA); });
			}
		}

		for (auto& pair : m_prevCollisionPairs)
		{
			if (currPairs.find(pair) == currPairs.end())
			{
				auto* compA = FindComponent(pair.first->body);
				auto* compB = FindComponent(pair.second->body);

				if (compA && compA->GetOwner())
					compA->GetOwner()->ForEachComponent([compB](Component* c) { c->OnCollisionExit(compB); });
				if (compB && compB->GetOwner())
					compB->GetOwner()->ForEachComponent([compA](Component* c) { c->OnCollisionExit(compA); });
			}
		}

		m_prevCollisionPairs = std::move(currPairs);
	}

	void PhysicsManager::ProcessTriggerCallbacks()
	{
		std::set<TriggerPair> currPairs;
		for (auto& ct : m_world.GetTriggers())
		{
			if (!ct.colA || !ct.colB)
				continue;
			currPairs.insert(MakePair(ct.colA, ct.colB));
		}

		for (auto& pair : currPairs)
		{
			auto* compA = FindComponent(pair.first->body);
			auto* compB = FindComponent(pair.second->body);

			if (m_prevTriggerPairs.find(pair) == m_prevTriggerPairs.end())
			{
				// Enter
				if (compA && compA->GetOwner())
					compA->GetOwner()->ForEachComponent([compB](Component* c) { c->OnTriggerEnter(compB); });
				if (compB && compB->GetOwner())
					compB->GetOwner()->ForEachComponent([compA](Component* c) { c->OnTriggerEnter(compA); });
			}
			else
			{
				// Stay
				if (compA && compA->GetOwner())
					compA->GetOwner()->ForEachComponent([compB](Component* c) { c->OnTriggerStay(compB); });
				if (compB && compB->GetOwner())
					compB->GetOwner()->ForEachComponent([compA](Component* c) { c->OnTriggerStay(compA); });
			}
		}

		for (auto& pair : m_prevTriggerPairs)
		{
			if (currPairs.find(pair) == currPairs.end())
			{
				auto* compA = FindComponent(pair.first->body);
				auto* compB = FindComponent(pair.second->body);

				if (compA && compA->GetOwner())
					compA->GetOwner()->ForEachComponent([compB](Component* c) { c->OnTriggerExit(compB); });
				if (compB && compB->GetOwner())
					compB->GetOwner()->ForEachComponent([compA](Component* c) { c->OnTriggerExit(compA); });
			}
		}

		m_prevTriggerPairs = std::move(currPairs);
	}

	void PhysicsManager::Step(float dt)
	{
		// dt가 너무 크면 상한 제한 (창 드래그 등으로 수 초 쌓이는 것 방지)
		if (dt > m_maxAccum)
			dt = m_maxAccum;

		// 누적
		m_accumulator += dt;

		// 고정 간격만큼 반복 실행
		while (m_accumulator >= m_fixedDt)
		{
			m_world.Step(m_fixedDt);
			ProcessCollisionCallbacks();
			ProcessTriggerCallbacks();

	#ifdef _DEBUG
			// ── 감시 대상 바디의 충돌 속도 기록 ──
			if (m_watchedBody)
			{
				for (const auto& c : m_world.GetCollisions())
				{
					if (c.bodyA != m_watchedBody && c.bodyB != m_watchedBody)
						continue;

					Vec3 linVel = m_watchedBody->GetLinearVelocity();
					Vec3 angVel = m_watchedBody->GetAngularVelocity();
					float speed = Vec3Length(linVel);

					char buf[256];
					snprintf(buf, sizeof(buf),
						"vel=(%.2f, %.2f, %.2f) |v|=%.3f  angVel=(%.2f, %.2f, %.2f)  normal=(%.2f, %.2f, %.2f)  depth=%.4f",
						linVel.x, linVel.y, linVel.z, speed,
						angVel.x, angVel.y, angVel.z,
						c.normal.x, c.normal.y, c.normal.z,
						c.depth);

					m_collisionLog.push_back(buf);

					if (m_collisionLog.size() > MAX_LOG_ENTRIES)
						m_collisionLog.erase(m_collisionLog.begin());

					break;  // 프레임당 1회만 기록 (같은 바디 중복 방지)
				}
			}
#endif

			m_accumulator -= m_fixedDt;
		}

		// 남은 m_accumulator는 다음 프레임으로 이월
	}

	bool PhysicsManager::Raycast(const Ray& ray, RayHit& outHit) const
	{
		return m_world.Raycast(ray, outHit);
	}

	int PhysicsManager::RaycastAll(const Ray& ray, std::vector<RayHit>& outHits) const
	{
		return m_world.RaycastAll(ray, outHits);
	}

	void PhysicsManager::Clear()
	{
#ifdef _DEBUG
		m_watchedBody = nullptr;
		m_collisionLog.clear();
#endif
		m_prevCollisionPairs.clear();
		m_prevTriggerPairs.clear();
		m_bodyMap.clear();
		m_world.ClearRigidBody();
		m_world.ClearColliders();
	}

#ifdef _DEBUG
	void PhysicsManager::DrawConsole()
	{
		ImGui::Begin("Physics Console");

		// 상태 표시
		if (m_watchedBody)
			ImGui::TextColored(ImVec4(0.4f, 1.f, 0.4f, 1.f), "Watching body (entries: %d)", (int)m_collisionLog.size());
		else
			ImGui::TextDisabled("No body watched");

		if (ImGui::Button("Clear Log"))
			m_collisionLog.clear();

		// ── 파일 저장 ──
		ImGui::Separator();
		static char savePath[512] = "collision_log.txt";
		ImGui::InputText("Save Path", savePath, sizeof(savePath));
		ImGui::SameLine();
		if (ImGui::Button("Save"))
		{
			FILE* fp = nullptr;
			fopen_s(&fp, savePath, "w");
			if (fp)
			{
				for (size_t i = 0; i < m_collisionLog.size(); ++i)
					fprintf(fp, "[%04zu] %s\n", i, m_collisionLog[i].c_str());
				fclose(fp);
				m_lastSaveMessage = "Saved " + std::to_string(m_collisionLog.size()) + " entries.";
			}
			else
			{
				m_lastSaveMessage = "Failed to open: " + std::string(savePath);
			}
		}
		if (!m_lastSaveMessage.empty())
			ImGui::TextColored(ImVec4(1.f, 1.f, 0.4f, 1.f), "%s", m_lastSaveMessage.c_str());

		ImGui::Separator();

		// 로그 스크롤 영역
		ImGui::BeginChild("LogRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
		for (size_t i = 0; i < m_collisionLog.size(); ++i)
		{
			ImGui::TextUnformatted(m_collisionLog[i].c_str());
		}
		// 자동 스크롤 (새 항목 추가 시)
		if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 20.f)
			ImGui::SetScrollHereY(1.f);
		ImGui::EndChild();

		ImGui::End();
	}
#endif
}
