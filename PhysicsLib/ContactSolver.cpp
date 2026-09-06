#include "PhysicsPch.h"
#include "ContactSolver.h"
#include "Collider.h"
#include "RigidBody.h"


void ContactSolver::ComputeTangentBasis(const Vec3& normal, Vec3& t1, Vec3& t2)
{
	// normal과 가장 덜 평행한 축을 기준으로 직교 벡터 생성
	if (fabsf(normal.x) < 0.57735f)
		t1 = Vec3Cross(normal, Vec3(1.f, 0.f, 0.f));
	else
		t1 = Vec3Cross(normal, Vec3(0.f, 1.f, 0.f));

	t1 = Vec3Normalize(t1);
	t2 = Vec3Cross(normal, t1);
}

void ContactSolver::Resolve(std::vector<Contact>& contacts, float dt)
{
	if (contacts.empty())
		return;

	Presolve(contacts, dt);

	for (int iter = 0; iter < iterations; ++iter)
	{
		SolveVelocity();
		SolvePositionBias();
	}
}

void ContactSolver::Presolve(std::vector<Contact>& contacts, float dt)
{
	m_constraints.resize(contacts.size());

	for (size_t i = 0; i < m_constraints.size(); ++i)
	{
		Contact& ct = contacts[i];
		ContactConstraint& cc = m_constraints[i];

		cc.ct = &ct;

		RigidBody* a = ct.bodyA;
		RigidBody* b = ct.bodyB;

		cc.rA = ct.point - a->GetPosition();
		cc.rB = ct.point - b->GetPosition();

		//  Normal 유효질량
		Vec3 rAxN = Vec3Cross(cc.rA, ct.normal);
		Vec3 rBxN = Vec3Cross(cc.rB, ct.normal);
		Vec3 termA = Vec3Cross(Mat3MultiplyVec3(a->GetInvInertiaWorld(), rAxN), cc.rA);
		Vec3 termB = Vec3Cross(Mat3MultiplyVec3(b->GetInvInertiaWorld(), rBxN), cc.rB);

		float kNormal = a->GetInvMass() + b->GetInvMass()
			+ Vec3Dot(termA, ct.normal) + Vec3Dot(termB, ct.normal);

		cc.effectiveMassNormal = (kNormal > 1e-6f) ? (1.f / kNormal) : 0.f;

		// 상대 속도
		Vec3 vA = a->GetVelocityAtPoint(ct.point);
		Vec3 vB = b->GetVelocityAtPoint(ct.point);
		Vec3 vRelative = vB - vA;

		float closingVel = Vec3Dot(vRelative, ct.normal);

		//  Restitution bias
		cc.mixedRestitution = (a->GetRestitution() + b->GetRestitution()) * 0.5f;

		const float restitution_threshold = -0.2f; // 0.2 이상의 속도로 접근 중일 때 적용
		cc.restitutionBias = (closingVel < restitution_threshold)
			? cc.mixedRestitution * closingVel : 0.f;
		
		// Baumgarte bias (pseudo velocity 전용) 
		cc.baumgarteBias = (beta / dt) * fmaxf(ct.depth - slop, 0.f);

		// Tangent 2축 (normal 기반 직교 벡터)
		ComputeTangentBasis(ct.normal, cc.tangent1, cc.tangent2);

		// Tangent1 유효질량
		{
			Vec3 rAxT = Vec3Cross(cc.rA, cc.tangent1);
			Vec3 rBxT = Vec3Cross(cc.rB, cc.tangent1);
			Vec3 tA = Vec3Cross(Mat3MultiplyVec3(a->GetInvInertiaWorld(), rAxT), cc.rA);
			Vec3 tB = Vec3Cross(Mat3MultiplyVec3(b->GetInvInertiaWorld(), rBxT), cc.rB);

			float k = a->GetInvMass() + b->GetInvMass()
				+ Vec3Dot(tA, cc.tangent1) + Vec3Dot(tB, cc.tangent1);

			cc.effectiveMassT1 = (k > 1e-6f) ? (1.f / k) : 0.f;
		}

		// Tangent2 유효질량
		{
			Vec3 rAxT = Vec3Cross(cc.rA, cc.tangent2);
			Vec3 rBxT = Vec3Cross(cc.rB, cc.tangent2);
			Vec3 tA = Vec3Cross(Mat3MultiplyVec3(a->GetInvInertiaWorld(), rAxT), cc.rA);
			Vec3 tB = Vec3Cross(Mat3MultiplyVec3(b->GetInvInertiaWorld(), rBxT), cc.rB);

			float k = a->GetInvMass() + b->GetInvMass()
				+ Vec3Dot(tA, cc.tangent2) + Vec3Dot(tB, cc.tangent2);

			cc.effectiveMassT2 = (k > 1e-6f) ? (1.f / k) : 0.f;
		}

		cc.mixedFriction = sqrtf(a->GetFriction() * b->GetFriction());

		// 누적 초기화
		cc.accumulatedImpulseN  = 0.f;
		cc.accumulatedImpulseT1 = 0.f;
		cc.accumulatedImpulseT2 = 0.f;
		cc.accumulatedPseudoN   = 0.f;
	}
}

void ContactSolver::SolveVelocity()
{
	for (auto& cc : m_constraints)
	{
		RigidBody* a = cc.ct->bodyA;
		RigidBody* b = cc.ct->bodyB;

		const Vec3& normal = cc.ct->normal;

		// Normal: 실제 속도 (restitution만, Baumgarte 없음)

		Vec3 vA = a->GetVelocityAtPoint(cc.ct->point);
		Vec3 vB = b->GetVelocityAtPoint(cc.ct->point);
		Vec3 vRelative = vB - vA;

		float vN = Vec3Dot(vRelative, normal);
		// restitution bias: 반대 방향으로 튕겨나가게 할 수 있게 하는 요소
		float lambdaN = -(vN + cc.restitutionBias) * cc.effectiveMassNormal;

		// 누적된 impulse가 0 이상으로 설정되기 위한 계산, 당기는 힘은 있을 수 없다
		float oldAccumN = cc.accumulatedImpulseN;
		cc.accumulatedImpulseN = fmaxf(oldAccumN + lambdaN, 0.f);
		lambdaN = cc.accumulatedImpulseN - oldAccumN;

		Vec3 impulseN = normal * lambdaN;
		a->ApplyImpulseAtPoint(-impulseN, cc.ct->point);
		b->ApplyImpulseAtPoint(impulseN, cc.ct->point);

		// Friction Tangent1
		vA = a->GetVelocityAtPoint(cc.ct->point);
		vB = b->GetVelocityAtPoint(cc.ct->point);
		vRelative = vB - vA;

		float vT1 = Vec3Dot(vRelative, cc.tangent1);
		float lambdaT1 = -vT1 * cc.effectiveMassT1;

		float frictionLimit = cc.mixedFriction * cc.accumulatedImpulseN;
		float oldAccumT1 = cc.accumulatedImpulseT1;
		cc.accumulatedImpulseT1 = fmaxf(-frictionLimit, fminf(oldAccumT1 + lambdaT1, frictionLimit));
		lambdaT1 = cc.accumulatedImpulseT1 - oldAccumT1;

		Vec3 impulseT1 = cc.tangent1 * lambdaT1;
		a->ApplyImpulseAtPoint(-impulseT1, cc.ct->point);
		b->ApplyImpulseAtPoint(impulseT1, cc.ct->point);

		//  Friction Tangent2
		vA = a->GetVelocityAtPoint(cc.ct->point);
		vB = b->GetVelocityAtPoint(cc.ct->point);
		vRelative = vB - vA;

		float vT2 = Vec3Dot(vRelative, cc.tangent2);
		float lambdaT2 = -vT2 * cc.effectiveMassT2;

		float oldAccumT2 = cc.accumulatedImpulseT2;
		cc.accumulatedImpulseT2 = fmaxf(-frictionLimit, fminf(oldAccumT2 + lambdaT2, frictionLimit));
		lambdaT2 = cc.accumulatedImpulseT2 - oldAccumT2;

		Vec3 impulseT2 = cc.tangent2 * lambdaT2;
		a->ApplyImpulseAtPoint(-impulseT2, cc.ct->point);
		b->ApplyImpulseAtPoint(impulseT2, cc.ct->point);
	}
}

void ContactSolver::SolvePositionBias()
{
	for (auto& cc : m_constraints)
	{
		RigidBody* a = cc.ct->bodyA;
		RigidBody* b = cc.ct->bodyB;
	
		const Vec3& normal = cc.ct->normal;
	
		// Position correction: pseudo velocity만 사용
		Vec3 pseudoVA = a->GetPseudoVelocityAtPoint(cc.ct->point);
		Vec3 pseudoVB = b->GetPseudoVelocityAtPoint(cc.ct->point);
		Vec3 pseudoVRel = pseudoVB - pseudoVA;
	
		float pseudoVN = Vec3Dot(pseudoVRel, normal);
		float lambdaPos = -(pseudoVN + cc.baumgarteBias) * cc.effectiveMassNormal;
	
		float oldAccumP = cc.accumulatedPseudoN;
		cc.accumulatedPseudoN = fmaxf(oldAccumP + lambdaPos, 0.f);
		lambdaPos = cc.accumulatedPseudoN - oldAccumP;
	
		Vec3 impulsePos = normal * lambdaPos;
		a->ApplyPseudoImpulseAtPoint(-impulsePos, cc.ct->point);
		b->ApplyPseudoImpulseAtPoint(impulsePos, cc.ct->point);
	}
}
