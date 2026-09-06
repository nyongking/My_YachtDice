#pragma once

#include "Component.h"
#include "RigidBody.h"
#include "Collider.h"
namespace GameEngine
{
	class RigidBodyComponent : public Component
	{
	public:
		RigidBodyComponent()  = default;
		~RigidBodyComponent() = default;

	public:
		void Awake()              override;
		void Start()              override;
		void Update(float dt)     override;
		void LateUpdate(float dt) override;
		void OnDestroy()          override;

		// 직렬화
		std::string GetTypeName()            const override { return "RigidBodyComponent"; }
		MyJson      Serialize()              const override;
		void        Deserialize(const MyJson& j)   override;

		// ── Body 설정 ──
		void SetBodyType(RigidBody::BodyType type);
		void SetMass(float mass);
		void SetRestitution(float r);
		void SetFriction(float f);
		void SetLinearDamping(float d);
		void SetAngularDamping(float d);

		// ── Collider 설정 (단일 — 기존 호환, ClearColliders + Add) ──
		void SetSphereCollider(float radius);
		void SetBoxCollider(const float3& halfExtents);
		void SetPlaneCollider(const float3& normal, float dist);

		// ── Collider 누적 추가 ──
		void AddSphereCollider(float radius,
			const float3& posOffset = { 0,0,0 }, const float3& rotOffsetEuler = { 0,0,0 });
		void AddBoxCollider(const float3& halfExtents,
			const float3& posOffset = { 0,0,0 }, const float3& rotOffsetEuler = { 0,0,0 });
		void AddPlaneCollider(const float3& normal, float dist,
			const float3& posOffset = { 0,0,0 });
		void ClearColliders();

		void SetAllCollidersEnabled(bool enabled);
		void SetColliderEnabled(bool enabled, int index);

		void SetAllTrigger(bool trigger);
		void SetTrigger(bool trigger, int index);
		void SetCollisionLayer(unsigned int layer, unsigned int mask);

		// ── Transform 직접 설정 (텔레포트) ──
		void SetPosition(const float3& pos);
		void SetRotation(const float3& eulerDeg);

		// ── Force API (Dynamic body 전용) ──
		void ApplyForce(const float3& force);
		void ApplyForceAtPoint(const float3& force, const float3& point);
		void ApplyImpulse(const float3& impulse, const float3& point);

		// ── Orientation Injection ──
		void ApplyOrientationInjection(const float3& upDir, float kp, float kd);
		void ClearOrientationInjection();

		// ── Sleep API ──
		bool IsSleeping() const { return m_body && m_body->IsSleeping(); }
		void WakeUp()           { if (m_body) m_body->WakeUp(); }
		void ForceSleep()       { if (m_body && m_body->IsDynamic()) m_body->SetSleeping(true); }

		// ── 조회 ──
		RigidBody*    GetRigidBody() const { return m_body.get(); }
		Collider*     GetCollider()  const { return m_colliders.empty() ? nullptr : m_colliders[0].get(); }
		const std::vector<std::unique_ptr<Collider>>& GetColliders() const { return m_colliders; }
		size_t        GetColliderCount() const { return m_colliders.size(); }

#ifdef _DEBUG
		void OnInspectorGUI() override;
#endif

	private:
		void SyncTransformToBody();   // Transform → RigidBody (초기화 / Kinematic)
		void SyncBodyToTransform();   // RigidBody → Transform (Dynamic 시뮬레이션 결과)

		void RegisterToWorld();
		void UnregisterFromWorld();
		void AddColliderInternal(std::unique_ptr<Collider> col,
			const float3& posOffset, const float3& rotOffsetEuler);
		void UpdateInertia();         // collider 모양 + mass 기반 관성 텐서 재계산

	private:
		std::unique_ptr<RigidBody>                m_body;
		std::vector<std::unique_ptr<Collider>>    m_colliders;
		bool                                      m_registered = false;

		// 직렬화용 캐시 (Deserialize 후 Awake/Start에서 적용)
		RigidBody::BodyType m_initBodyType    = RigidBody::BodyType::Dynamic;
		float               m_initMass        = 1.f;
		float               m_initRestitution = 0.3f;
		float               m_initFriction    = 0.5f;
		float               m_initLinearDamp  = 0.01f;
		float               m_initAngularDamp = 0.05f;

		// Kinematic velocity 계산용
		float               m_kinematicDt     = 0.f;

		// 콜라이더들의 질량 중심 오프셋 (body origin 기준 로컬 좌표)
		Vec3                m_localCOM;
	};
}
