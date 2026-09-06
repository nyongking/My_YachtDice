#include "GameEnginePch.h"
#include "RigidBodyComponent.h"
#include "PhysicsConvert.h"
#include "PhysicsManager.h"
#include "EngineGlobal.h"
#include "GameObject.h"
#include "Transform.h"
#include "Operation.h"

#ifdef _DEBUG
#include "Imgui/imgui.h"
#include "RenderItem.h"
#include "RenderPipeline.h"
#include "RenderPassBase.h"
#include "CameraManager.h"
#include "CameraComponent.h"
#include "GeometryManager.h"
#include "Material.h"
#endif

namespace GameEngine
{
	// layerMask JSON 역직렬화: "all" | [0,1,2,...] | 정수
	static unsigned int ParseLayerMask(const MyJson& val)
	{
		if (val.is_string())
		{
			if (val.get<std::string>() == "all")
				return 0xFFFFFFFF;
			return 0xFFFFFFFF;  // 알 수 없는 문자열은 all로 처리
		}
		if (val.is_array())
		{
			unsigned int mask = 0;
			for (auto& elem : val)
			{
				unsigned int bit = elem.get<unsigned int>();
				if (bit < 32)
					mask |= (1u << bit);
			}
			return mask;
		}
		// 구 포맷 호환: 정수
		return val.get<unsigned int>();
	}

	// ── Lifecycle ───────────────────────────────────

	void RigidBodyComponent::Awake()
	{
		m_body = std::make_unique<RigidBody>();

		m_body->SetBodyType(m_initBodyType);
		m_body->SetMass(m_initMass);
		m_body->SetRestitution(m_initRestitution);
		m_body->SetFriction(m_initFriction);
		m_body->SetLinearDamping(m_initLinearDamp);
		m_body->SetAngularDamping(m_initAngularDamp);
	}

	void RigidBodyComponent::Start()
	{
		SyncTransformToBody();
		RegisterToWorld();
	}

	void RigidBodyComponent::Update(float dt)
	{
		if (!m_body) return;

		if (m_body->IsDynamic())
		{
			if (!m_body->IsSleeping())
				SyncBodyToTransform();
		}
		else if (m_body->IsKinematic())
		{
			m_kinematicDt = dt;
			SyncTransformToBody();
		}
	}

	void RigidBodyComponent::LateUpdate(float dt)
	{
#ifdef _DEBUG
		if (!GDebugDrawColliders || !GDebugWireframeMaterial) return;
		if (m_colliders.empty()) return;

		auto* cam = CameraManager::GetInstance().GetMainCamera();
		if (!cam || !cam->GetViewProj()) return;

		Transform* tr = GetOwner()->GetTransform();

		using namespace DirectX;

		float3 basePos = tr->GetPosition();
		float3 rot     = tr->GetRotation();
		float toRad    = XM_PI / 180.f;
		XMMATRIX bodyR = XMMatrixRotationX(rot.x * toRad)
		               * XMMatrixRotationY(rot.y * toRad)
		               * XMMatrixRotationZ(rot.z * toRad);

		for (auto& col : m_colliders)
		{
			// posOffset을 body 회전 기준으로 월드 오프셋 계산
			float3 off = ToEngine(col->posOffset);
			XMVECTOR worldOff = XMVector3TransformNormal(
				XMVectorSet(off.x, off.y, off.z, 0.f), bodyR);
			XMFLOAT3 wo;
			XMStoreFloat3(&wo, worldOff);
			float3 pos = { basePos.x + wo.x, basePos.y + wo.y, basePos.z + wo.z };

			// rotOffset 적용: collider 로컬 회전 * body 회전
			float3 colRotEuler = PhysicsQuatToEuler(col->rotOffset);
			XMMATRIX colR = XMMatrixRotationX(colRotEuler.x * toRad)
			              * XMMatrixRotationY(colRotEuler.y * toRad)
			              * XMMatrixRotationZ(colRotEuler.z * toRad);
			XMMATRIX R = colR * bodyR;

			bool isTrigger = col->isTrigger;
			Render::Geometry* wireGeo = nullptr;
			float4x4 wireWorld;

			if (col->type == Collider::ColliderType::Sphere)
			{
				auto* sc = static_cast<SphereCollider*>(col.get());
				wireGeo = GGeometryManager->Get(isTrigger ? "WireSphereTrigger" : "WireSphere");

				float r = sc->radius;
				XMMATRIX S = XMMatrixScaling(r, r, r);
				XMMATRIX T = XMMatrixTranslation(pos.x, pos.y, pos.z);
				XMStoreFloat4x4(&wireWorld, S * R * T);
			}
			else if (col->type == Collider::ColliderType::Box)
			{
				auto* bc = static_cast<BoxCollider*>(col.get());
				wireGeo = GGeometryManager->Get(isTrigger ? "WireBoxTrigger" : "WireBox");

				float3 he = ToEngine(bc->halfExtents);
				XMMATRIX S = XMMatrixScaling(he.x * 2.f, he.y * 2.f, he.z * 2.f);
				XMMATRIX T = XMMatrixTranslation(pos.x, pos.y, pos.z);
				XMStoreFloat4x4(&wireWorld, S * R * T);
			}
			else if (col->type == Collider::ColliderType::Plane)
			{
				wireGeo = GGeometryManager->Get(isTrigger ? "WirePlaneTrigger" : "WirePlane");

				XMMATRIX T = XMMatrixTranslation(pos.x, pos.y, pos.z);
				XMStoreFloat4x4(&wireWorld, R * T);
			}

			if (!wireGeo) continue;

			Render::RenderCommand cmd;
			cmd.geometry = wireGeo;
			cmd.material = GDebugWireframeMaterial;
			cmd.world    = wireWorld;
			cmd.viewProj = *cam->GetViewProj();

			Render::RenderPipeline::GetInstance().Submit(
				Render::RenderPassBase::Layer::Effect, cmd);
		}
#endif
	}

	void RigidBodyComponent::OnDestroy()
	{
		UnregisterFromWorld();
		m_colliders.clear();
		m_body.reset();
	}

	void RigidBodyComponent::UpdateInertia()
	{
		m_localCOM.MakeZero();

		if (!m_body || !m_body->IsDynamic() || m_body->GetInvMass() <= 0.f)
			return;
		if (m_colliders.empty())
			return;

		float totalMass = 1.f / m_body->GetInvMass();
		float massPerCol = totalMass / static_cast<float>(m_colliders.size());

		// ── 1단계: 질량 중심(CoM) 계산 ──
		Vec3 com;
		com.MakeZero();
		int validCount = 0;

		for (auto& col : m_colliders)
		{
			if (col->type == Collider::ColliderType::Plane)
				continue;
			com += col->posOffset * massPerCol;
			++validCount;
		}

		if (validCount > 0)
			com *= (1.f / totalMass);

		m_localCOM = com;

		// ── 2단계: CoM 기준 관성 텐서 (평행축 정리) ──
		Mat3 totalInertia;
		totalInertia.MakeZero();

		for (auto& col : m_colliders)
		{
			Mat3 localInertia;
			localInertia.MakeZero();

			if (col->type == Collider::ColliderType::Sphere)
			{
				auto* sc = static_cast<SphereCollider*>(col.get());
				float val = (2.f / 5.f) * massPerCol * sc->radius * sc->radius;
				localInertia.m[0][0] = val;
				localInertia.m[1][1] = val;
				localInertia.m[2][2] = val;
			}
			else if (col->type == Collider::ColliderType::Box)
			{
				auto* bc = static_cast<BoxCollider*>(col.get());
				float wx = 2.f * bc->halfExtents.x;
				float hy = 2.f * bc->halfExtents.y;
				float dz = 2.f * bc->halfExtents.z;
				float k = massPerCol / 12.f;
				localInertia.m[0][0] = k * (hy * hy + dz * dz);
				localInertia.m[1][1] = k * (wx * wx + dz * dz);
				localInertia.m[2][2] = k * (wx * wx + hy * hy);
			}
			else
			{
				continue;
			}

			// rotOffset 회전: I' = R * I * R^T
			Quat rq = col->rotOffset;
			bool hasRotation = (rq.x * rq.x + rq.y * rq.y + rq.z * rq.z) > 1e-6f;
			if (hasRotation)
			{
				Mat3 R  = QuatToMat3(rq);
				Mat3 Rt = Mat3Transpose(R);
				localInertia = R * localInertia * Rt;
			}

			// 평행축 정리: CoM 기준 오프셋 사용
			Vec3 d = col->posOffset - com;
			float d2 = Vec3Dot(d, d);

			for (int r = 0; r < 3; r++)
			{
				for (int c = 0; c < 3; c++)
				{
					float kronecker = (r == c) ? 1.f : 0.f;
					float steiner = massPerCol * (d2 * kronecker - d.v[r] * d.v[c]);
					totalInertia.m[r][c] += localInertia.m[r][c] + steiner;
				}
			}
		}

		m_body->SetInertia(totalInertia);
	}

	// ── Transform 동기화 ────────────────────────────

	void RigidBodyComponent::SyncTransformToBody()
	{
		if (!m_body) return;

		Transform* tr = GetOwner()->GetTransform();
		Quat newOri = EulerToPhysicsQuat(tr->GetRotation());
		// body position = Transform origin + 회전된 CoM offset
		Vec3 newPos = ToPhysics(tr->GetPosition()) + QuatRotateVec3(newOri, m_localCOM);

		// Kinematic: 이전 위치와의 차이로 속도를 계산해야
		// ContactSolver가 Dynamic 바디를 밀어낼 수 있다
		if (m_body->IsKinematic() && m_kinematicDt > 0.f)
		{
			Vec3 oldPos = m_body->GetPosition();
			Vec3 vel    = (newPos - oldPos) * (1.f / m_kinematicDt);
			m_body->SetLinearVelocity(vel);

			// 각속도: 현재→목표 쿼터니언 차이로 계산
			Quat oldOri = m_body->GetOrientation();
			Quat delta  = QuatMultiply(newOri, QuatConjugate(oldOri));
			if (delta.w < 0.f)
			{
				delta.x = -delta.x; delta.y = -delta.y;
				delta.z = -delta.z; delta.w = -delta.w;
			}
			Vec3 angVel = Vec3(delta.x, delta.y, delta.z) * (2.f / m_kinematicDt);
			m_body->SetAngularVelocity(angVel);
		}

		m_body->SetPosition(newPos);
		m_body->SetOrientation(newOri);
	}

	void RigidBodyComponent::SyncBodyToTransform()
	{
		if (!m_body) return;

		Transform* tr = GetOwner()->GetTransform();
		Quat ori = m_body->GetOrientation();
		// Transform origin = body position(CoM) - 회전된 CoM offset
		Vec3 transformPos = m_body->GetPosition() - QuatRotateVec3(ori, m_localCOM);
		tr->SetPosition(ToEngine(transformPos));
		tr->SetRotation(PhysicsQuatToEuler(ori));
	}

	// ── PhysicsWorld 등록/해제 ──────────────────────

	void RigidBodyComponent::RegisterToWorld()
	{
		if (m_registered || !GPhysicsManager) return;

		PhysicsWorld& world = GPhysicsManager->GetWorld();

		if (m_body)
		{
			world.AddRigidBody(m_body.get());
			GPhysicsManager->RegisterBodyMapping(m_body.get(), this);
		}

		for (auto& col : m_colliders)
			world.AddCollider(col.get());

		m_registered = true;
	}

	void RigidBodyComponent::UnregisterFromWorld()
	{
		if (!m_registered || !GPhysicsManager) return;

		PhysicsWorld& world = GPhysicsManager->GetWorld();

		for (auto& col : m_colliders)
			world.RemoveCollider(col.get());

		if (m_body)
		{
			GPhysicsManager->UnregisterBodyMapping(m_body.get());
			world.RemoveRigidBody(m_body.get());
		}

		m_registered = false;
	}

	void RigidBodyComponent::AddColliderInternal(std::unique_ptr<Collider> col,
		const float3& posOffset, const float3& rotOffsetEuler)
	{
		col->body      = m_body.get();
		col->posOffset = ToPhysics(posOffset);
		col->rotOffset = EulerToPhysicsQuat(rotOffsetEuler);

		if (m_registered && GPhysicsManager)
			GPhysicsManager->GetWorld().AddCollider(col.get());

		m_colliders.push_back(std::move(col));
	}

	// ── Body 설정 ───────────────────────────────────

	void RigidBodyComponent::SetBodyType(RigidBody::BodyType type)
	{
		m_initBodyType = type;
		if (m_body) m_body->SetBodyType(type);
	}

	void RigidBodyComponent::SetMass(float mass)
	{
		m_initMass = mass;
		if (m_body) m_body->SetMass(mass);
		UpdateInertia();
	}

	void RigidBodyComponent::SetRestitution(float r)
	{
		m_initRestitution = r;
		if (m_body) m_body->SetRestitution(r);
	}

	void RigidBodyComponent::SetFriction(float f)
	{
		m_initFriction = f;
		if (m_body) m_body->SetFriction(f);
	}

	void RigidBodyComponent::SetLinearDamping(float d)
	{
		m_initLinearDamp = d;
		if (m_body) m_body->SetLinearDamping(d);
	}

	void RigidBodyComponent::SetAngularDamping(float d)
	{
		m_initAngularDamp = d;
		if (m_body) m_body->SetAngularDamping(d);
	}

	// ── Collider 누적 추가 ─────────────────────────

	void RigidBodyComponent::AddSphereCollider(float radius,
		const float3& posOffset, const float3& rotOffsetEuler)
	{
		auto col    = std::make_unique<SphereCollider>();
		col->radius = radius;
		AddColliderInternal(std::move(col), posOffset, rotOffsetEuler);
		UpdateInertia();
	}

	void RigidBodyComponent::AddBoxCollider(const float3& halfExtents,
		const float3& posOffset, const float3& rotOffsetEuler)
	{
		auto col         = std::make_unique<BoxCollider>();
		col->halfExtents = ToPhysics(halfExtents);
		AddColliderInternal(std::move(col), posOffset, rotOffsetEuler);
		UpdateInertia();
	}

	void RigidBodyComponent::AddPlaneCollider(const float3& normal, float dist,
		const float3& posOffset)
	{
		auto col    = std::make_unique<PlaneCollider>();
		col->normal = ToPhysics(normal);
		col->dist   = dist;
		AddColliderInternal(std::move(col), posOffset, { 0,0,0 });
	}

	void RigidBodyComponent::ClearColliders()
	{
		if (m_registered && GPhysicsManager)
		{
			for (auto& col : m_colliders)
				GPhysicsManager->GetWorld().RemoveCollider(col.get());
		}
		m_colliders.clear();
	}

	// ── Collider 설정 (단일 — 기존 호환) ───────────

	void RigidBodyComponent::SetSphereCollider(float radius)
	{
		ClearColliders();
		AddSphereCollider(radius);
	}

	void RigidBodyComponent::SetBoxCollider(const float3& halfExtents)
	{
		ClearColliders();
		AddBoxCollider(halfExtents);
	}

	void RigidBodyComponent::SetPlaneCollider(const float3& normal, float dist)
	{
		ClearColliders();
		AddPlaneCollider(normal, dist);
	}

	void RigidBodyComponent::SetAllCollidersEnabled(bool enabled)
	{
		for (auto& col : m_colliders)
			col->enabled = enabled;
	}

	void RigidBodyComponent::SetColliderEnabled(bool enabled, int index)
	{
		if (index < 0 || index >= static_cast<int>(m_colliders.size()))
			return;

		m_colliders[index]->enabled = enabled;
	}

	void RigidBodyComponent::SetAllTrigger(bool trigger)
	{
		for (auto& col : m_colliders)
			col->isTrigger = trigger;
	}

	void RigidBodyComponent::SetTrigger(bool trigger, int index)
	{
		if (index >= static_cast<int>(m_colliders.size()))
			return;

		m_colliders[index]->isTrigger = trigger;
	}

	void RigidBodyComponent::SetCollisionLayer(unsigned int layer, unsigned int mask)
	{
		for (auto& col : m_colliders)
		{
			col->layer     = layer;
			col->layerMask = mask;
		}
	}

	// ── Transform 직접 설정 ────────────────────────

	void RigidBodyComponent::SetPosition(const float3& pos)
	{
		if (!m_body) return;
		Quat ori = m_body->GetOrientation();
		m_body->SetPosition(ToPhysics(pos) + QuatRotateVec3(ori, m_localCOM));
		GetOwner()->GetTransform()->SetPosition(pos);
	}

	void RigidBodyComponent::SetRotation(const float3& eulerDeg)
	{
		if (!m_body) return;
		Quat newOri = EulerToPhysicsQuat(eulerDeg);
		// 회전이 바뀌면 CoM의 월드 위치도 바뀜 → body position 재계산
		Vec3 transformPos = ToPhysics(GetOwner()->GetTransform()->GetPosition());
		m_body->SetPosition(transformPos + QuatRotateVec3(newOri, m_localCOM));
		m_body->SetOrientation(newOri);
		GetOwner()->GetTransform()->SetRotation(eulerDeg);
	}

	// ── Force API ──────────────────────────────────

	void RigidBodyComponent::ApplyForce(const float3& force)
	{
		if (m_body && m_body->IsDynamic())
			m_body->ApplyForce(ToPhysics(force));
	}

	void RigidBodyComponent::ApplyForceAtPoint(const float3& force, const float3& point)
	{
		if (m_body && m_body->IsDynamic())
			m_body->ApplyForceAtPoint(ToPhysics(force), ToPhysics(point));
	}

	void RigidBodyComponent::ApplyImpulse(const float3& impulse, const float3& point)
	{
		if (m_body && m_body->IsDynamic())
			m_body->ApplyImpulseAtPoint(ToPhysics(impulse), ToPhysics(point));
	}

	void RigidBodyComponent::ApplyOrientationInjection(const float3& dir, float kp, float kd)
	{
		if (m_body && m_body->IsDynamic())
		{
			m_body->SetOrientationTarget(EulerToPhysicsQuat(dir), kp, kd);
		}
	}

	void RigidBodyComponent::ClearOrientationInjection()
	{
		if (m_body && m_body->IsDynamic())
			m_body->ClearOrientationTarget();
	}

	// ── Inspector ──────────────────────────────────

#ifdef _DEBUG
	void RigidBodyComponent::OnInspectorGUI()
	{
		// Position / Rotation 직접 설정
		if (m_body)
		{
			Transform* tr = GetOwner()->GetTransform();
			float3 pos = tr->GetPosition();
			float3 rot = tr->GetRotation();
			if (ImGui::DragFloat3("Position", &pos.x, 0.1f))
				SetPosition(pos);
			if (ImGui::DragFloat3("Rotation", &rot.x, 0.1f))
				SetRotation(rot);
			ImGui::Separator();
		}

		// Body Type
		const char* bodyTypes[] = { "Static", "Kinematic", "Dynamic" };
		int btIdx = static_cast<int>(m_initBodyType);
		if (ImGui::Combo("Body Type", &btIdx, bodyTypes, 3))
			SetBodyType(static_cast<RigidBody::BodyType>(btIdx));

		if (m_initBodyType == RigidBody::BodyType::Dynamic)
		{
			if (ImGui::DragFloat("Mass", &m_initMass, 0.1f, 0.01f, 10000.f)) SetMass(m_initMass);
		}

		if (ImGui::DragFloat("Restitution",    &m_initRestitution, 0.01f, 0.f, 1.f))   SetRestitution(m_initRestitution);
		if (ImGui::DragFloat("Friction",        &m_initFriction,    0.01f, 0.f, 2.f))   SetFriction(m_initFriction);
		if (ImGui::DragFloat("Linear Damping",  &m_initLinearDamp,  0.001f, 0.f, 1.f))  SetLinearDamping(m_initLinearDamp);
		if (ImGui::DragFloat("Angular Damping", &m_initAngularDamp, 0.001f, 0.f, 1.f))  SetAngularDamping(m_initAngularDamp);

		// ── Colliders ──
		ImGui::Separator();
		ImGui::Text("Colliders (%d)", static_cast<int>(m_colliders.size()));

		int removeIdx = -1;
		for (int i = 0; i < static_cast<int>(m_colliders.size()); ++i)
		{
			ImGui::PushID(i);
			auto& col = m_colliders[i];

			const char* typeName = "Unknown";
			if (col->type == Collider::ColliderType::Sphere) typeName = "Sphere";
			else if (col->type == Collider::ColliderType::Box) typeName = "Box";
			else if (col->type == Collider::ColliderType::Plane) typeName = "Plane";

			bool open = ImGui::TreeNode("", "[%d] %s", i, typeName);
			ImGui::SameLine(ImGui::GetWindowWidth() - 60.f);
			if (ImGui::SmallButton("X"))
				removeIdx = i;

			if (open)
			{
				if (col->type == Collider::ColliderType::Sphere)
				{
					auto* sc = static_cast<SphereCollider*>(col.get());
					if (ImGui::DragFloat("Radius", &sc->radius, 0.01f, 0.01f, 1000.f))
						UpdateInertia();
				}
				else if (col->type == Collider::ColliderType::Box)
				{
					auto* bc = static_cast<BoxCollider*>(col.get());
					if (ImGui::DragFloat3("Half Extents", &bc->halfExtents.x, 0.01f, 0.01f, 1000.f))
						UpdateInertia();
				}
				else if (col->type == Collider::ColliderType::Plane)
				{
					auto* pc = static_cast<PlaneCollider*>(col.get());
					ImGui::DragFloat3("Normal", &pc->normal.x, 0.01f, -1.f, 1.f);
					ImGui::DragFloat("Distance", &pc->dist, 0.1f);
				}

				ImGui::DragFloat3("Pos Offset", &col->posOffset.x, 0.1f);

				float3 rotEuler = PhysicsQuatToEuler(col->rotOffset);
				if (ImGui::DragFloat3("Rot Offset", &rotEuler.x, 1.f))
					col->rotOffset = EulerToPhysicsQuat(rotEuler);

				bool colEnabled = col->enabled;
				if (ImGui::Checkbox("Enabled", &colEnabled))
					col->enabled = colEnabled;

				bool trigger = col->isTrigger;
				if (ImGui::Checkbox("Is Trigger", &trigger))
					col->isTrigger = trigger;

				ImGui::TreePop();
			}
			ImGui::PopID();
		}

		// 콜라이더 제거
		if (removeIdx >= 0 && removeIdx < static_cast<int>(m_colliders.size()))
		{
			if (m_registered && GPhysicsManager)
				GPhysicsManager->GetWorld().RemoveCollider(m_colliders[removeIdx].get());
			m_colliders.erase(m_colliders.begin() + removeIdx);
			UpdateInertia();
		}

		// 콜라이더 추가
		static int s_addShape = 0;
		const char* addShapes[] = { "Sphere", "Box", "Plane" };
		ImGui::Combo("##AddShape", &s_addShape, addShapes, 3);
		ImGui::SameLine();
		if (ImGui::Button("Add Collider"))
		{
			switch (s_addShape)
			{
			case 0: AddSphereCollider(0.5f);                   break;
			case 1: AddBoxCollider({ 0.5f, 0.5f, 0.5f });     break;
			case 2: AddPlaneCollider({ 0.f, 1.f, 0.f }, 0.f); break;
			}
		}

		if (ImGui::Button("Clear All Colliders"))
			ClearColliders();

		// ── Orientation Injection ──
		if (m_body && m_body->IsDynamic())
		{
			ImGui::Separator();
			ImGui::Text("Orientation Injection");

			static float s_kp = 50.f;
			static float s_kd = 5.f;

			// P/PD 비교용 퀵 프리셋 — 같은 Kp 기준으로 Kd만 스위칭
			ImGui::TextUnformatted("Mode Preset");
			if (ImGui::Button("P", ImVec2(60, 0)))  { s_kd = 0.f; }
			ImGui::SameLine();
			if (ImGui::Button("PD", ImVec2(60, 0))) { s_kd = 5.f; }

			ImGui::DragFloat("Kp (Stiffness)", &s_kp, 1.f, 0.f, 500.f);
			ImGui::DragFloat("Kd (Damping)",   &s_kd, 0.1f, 0.f, 50.f);

			// 6방향: 각 면이 위를 향하는 오일러 각도
			struct DirPreset { const char* label; float3 euler; };
			static const DirPreset presets[] = {
				{ "Up (+Y)",    {   0.f,  0.f,   0.f } },
				{ "Down (-Y)",  { 180.f,  0.f,   0.f } },
				{ "Left (-X)",  {   0.f,  0.f, -90.f } },
				{ "Right (+X)", {   0.f,  0.f,  90.f } },
				{ "Front (+Z)", {  90.f,  0.f,   0.f } },
				{ "Back (-Z)",  { -90.f,  0.f,   0.f } },
			};

			for (int i = 0; i < 6; ++i)
			{
				if (i == 2 || i == 4) ImGui::Separator();  // 그룹 구분
				if (ImGui::Button(presets[i].label, ImVec2(-1, 0)))
					ApplyOrientationInjection(presets[i].euler, s_kp, s_kd);
			}

			if (ImGui::Button("Clear", ImVec2(-1, 0)))
				ClearOrientationInjection();
		}

		// ── 충돌 속도 모니터링 ──
		ImGui::Separator();
		if (m_body && GPhysicsManager)
		{
			bool isWatched = (GPhysicsManager->GetWatchedBody() == m_body.get());
			if (isWatched)
			{
				if (ImGui::Button("Unwatch Collisions"))
					GPhysicsManager->ClearWatchedBody();
			}
			else
			{
				if (ImGui::Button("Watch Collisions"))
					GPhysicsManager->SetWatchedBody(m_body.get());
			}
		}
	}
#endif

	// ── 직렬화 ─────────────────────────────────────

	static const char* s_bodyTypeNames[] = { "Static", "Kinematic", "Dynamic" };

	static const char* ColliderTypeName(Collider::ColliderType t)
	{
		switch (t)
		{
		case Collider::ColliderType::Sphere: return "Sphere";
		case Collider::ColliderType::Box:    return "Box";
		case Collider::ColliderType::Plane:  return "Plane";
		default: return "None";
		}
	}

	MyJson RigidBodyComponent::Serialize() const
	{
		MyJson j;
		j["type"]          = GetTypeName();
		j["bodyType"]      = s_bodyTypeNames[static_cast<int>(m_initBodyType)];
		j["mass"]          = m_initMass;
		j["restitution"]   = m_initRestitution;
		j["friction"]      = m_initFriction;
		j["linearDamping"] = m_initLinearDamp;
		j["angularDamping"]= m_initAngularDamp;

		MyJson colArray = MyJson::array();
		for (auto& col : m_colliders)
		{
			MyJson cj;
			cj["shape"] = ColliderTypeName(col->type);

			if (col->type == Collider::ColliderType::Sphere)
			{
				auto* sc = static_cast<SphereCollider*>(col.get());
				cj["radius"] = sc->radius;
			}
			else if (col->type == Collider::ColliderType::Box)
			{
				auto* bc = static_cast<BoxCollider*>(col.get());
				cj["halfExtents"] = { bc->halfExtents.x, bc->halfExtents.y, bc->halfExtents.z };
			}
			else if (col->type == Collider::ColliderType::Plane)
			{
				auto* pc = static_cast<PlaneCollider*>(col.get());
				cj["normal"] = { pc->normal.x, pc->normal.y, pc->normal.z };
				cj["dist"]   = pc->dist;
			}

			cj["posOffset"] = { col->posOffset.x, col->posOffset.y, col->posOffset.z };

			float3 rotEuler = PhysicsQuatToEuler(col->rotOffset);
			cj["rotOffset"] = { rotEuler.x, rotEuler.y, rotEuler.z };

			cj["enabled"]   = col->enabled;
			cj["isTrigger"] = col->isTrigger;
			cj["layer"]     = col->layer;

			// layerMask → "all" 또는 배열 [0, 1, 2, ...]
			if (col->layerMask == 0xFFFFFFFF)
			{
				cj["layerMask"] = "all";
			}
			else
			{
				MyJson arr = MyJson::array();
				for (unsigned int bit = 0; bit < 32; ++bit)
				{
					if (col->layerMask & (1u << bit))
						arr.push_back(bit);
				}
				cj["layerMask"] = arr;
			}

			colArray.push_back(cj);
		}
		j["colliders"] = colArray;

		return j;
	}

	void RigidBodyComponent::Deserialize(const MyJson& j)
	{
		if (j.contains("bodyType"))
		{
			std::string bt = j["bodyType"].get<std::string>();
			if      (bt == "Static")    m_initBodyType = RigidBody::BodyType::Static;
			else if (bt == "Kinematic") m_initBodyType = RigidBody::BodyType::Kinematic;
			else                        m_initBodyType = RigidBody::BodyType::Dynamic;
		}

		if (j.contains("mass"))          m_initMass        = j["mass"];
		if (j.contains("restitution"))   m_initRestitution = j["restitution"];
		if (j.contains("friction"))      m_initFriction    = j["friction"];
		if (j.contains("linearDamping")) m_initLinearDamp  = j["linearDamping"];
		if (j.contains("angularDamping"))m_initAngularDamp = j["angularDamping"];

		// body가 이미 생성된 상태(Awake 이후)면 즉시 적용
		if (m_body)
		{
			m_body->SetBodyType(m_initBodyType);
			m_body->SetMass(m_initMass);
			m_body->SetRestitution(m_initRestitution);
			m_body->SetFriction(m_initFriction);
			m_body->SetLinearDamping(m_initLinearDamp);
			m_body->SetAngularDamping(m_initAngularDamp);
		}

		// 새 포맷: "colliders" 배열
		if (j.contains("colliders") && j["colliders"].is_array())
		{
			ClearColliders();
			for (auto& cj : j["colliders"])
			{
				std::string shape = cj.value("shape", "None");

				float3 posOff = { 0,0,0 };
				float3 rotOff = { 0,0,0 };
				if (cj.contains("posOffset"))
					posOff = { cj["posOffset"][0], cj["posOffset"][1], cj["posOffset"][2] };
				if (cj.contains("rotOffset"))
					rotOff = { cj["rotOffset"][0], cj["rotOffset"][1], cj["rotOffset"][2] };

				if (shape == "Sphere")
				{
					float r = cj.value("radius", 0.5f);
					AddSphereCollider(r, posOff, rotOff);
				}
				else if (shape == "Box")
				{
					float3 he = { 0.5f, 0.5f, 0.5f };
					if (cj.contains("halfExtents"))
						he = { cj["halfExtents"][0], cj["halfExtents"][1], cj["halfExtents"][2] };
					AddBoxCollider(he, posOff, rotOff);
				}
				else if (shape == "Plane")
				{
					float3 n = { 0.f, 1.f, 0.f };
					float  d = 0.f;
					if (cj.contains("normal"))
						n = { cj["normal"][0], cj["normal"][1], cj["normal"][2] };
					if (cj.contains("dist"))
						d = cj["dist"];
					AddPlaneCollider(n, d, posOff);
				}

				// 마지막에 추가된 콜라이더에 속성 적용
				if (!m_colliders.empty())
				{
					auto& last = m_colliders.back();
					if (cj.contains("enabled"))   last->enabled   = cj["enabled"];
					if (cj.contains("isTrigger")) last->isTrigger = cj["isTrigger"];
					if (cj.contains("layer"))     last->layer     = cj["layer"];
					if (cj.contains("layerMask")) last->layerMask = ParseLayerMask(cj["layerMask"]);
				}
			}
		}
		// 구 포맷 호환: "collider" 단일 객체
		else if (j.contains("collider"))
		{
			ClearColliders();
			const MyJson& cj = j["collider"];
			std::string shape = cj.value("shape", "None");

			float3 posOff = { 0,0,0 };
			if (cj.contains("posOffset"))
				posOff = { cj["posOffset"][0], cj["posOffset"][1], cj["posOffset"][2] };

			if (shape == "Sphere")
			{
				float r = cj.value("radius", 0.5f);
				AddSphereCollider(r, posOff);
			}
			else if (shape == "Box")
			{
				float3 he = { 0.5f, 0.5f, 0.5f };
				if (cj.contains("halfExtents"))
					he = { cj["halfExtents"][0], cj["halfExtents"][1], cj["halfExtents"][2] };
				AddBoxCollider(he, posOff);
			}
			else if (shape == "Plane")
			{
				float3 n = { 0.f, 1.f, 0.f };
				float  d = 0.f;
				if (cj.contains("normal"))
					n = { cj["normal"][0], cj["normal"][1], cj["normal"][2] };
				if (cj.contains("dist"))
					d = cj["dist"];
				AddPlaneCollider(n, d, posOff);
			}

			if (!m_colliders.empty())
			{
				auto& last = m_colliders.back();
				if (cj.contains("isTrigger")) last->isTrigger = cj["isTrigger"];
				if (cj.contains("layer"))     last->layer     = cj["layer"];
				if (cj.contains("layerMask")) last->layerMask = ParseLayerMask(cj["layerMask"]);
				if (cj.contains("enabled")) last->enabled = cj["enabled"];
			}
		}
	}

}
