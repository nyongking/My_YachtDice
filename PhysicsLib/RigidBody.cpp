#include "PhysicsPch.h"
#include "RigidBody.h"

// ── Sleep 시스템 ──

void RigidBody::SetSleeping(bool sleep)
{
	if (bodyType != BodyType::Dynamic) return;

	isSleeping = sleep;
	if (sleep)
	{
		linearVelocity.MakeZero();
		angularVelocity.MakeZero();
		pseudoLinearVelocity.MakeZero();
		pseudoAngularVelocity.MakeZero();
		forceAccum.MakeZero();
		torqueAccum.MakeZero();
		sleepTimer = 0.f;
	}
	else
	{
		sleepTimer = 0.f;
	}
}

void RigidBody::WakeUp()
{
	if (!isSleeping) return;
	if (bodyType != BodyType::Dynamic) return;
	SetSleeping(false);
}

void RigidBody::UpdateSleepState(float dt)
{
	if (bodyType != BodyType::Dynamic) return;
	if (isSleeping) return;
	if (hasOrientationTarget) return; // Orientation Injection 활성 시 sleep 거부

	float linSq = Vec3LengthSq(linearVelocity);
	float angSq = Vec3LengthSq(angularVelocity);

	if (linSq < SLEEP_LINEAR_THRESHOLD * SLEEP_LINEAR_THRESHOLD &&
		angSq < SLEEP_ANGULAR_THRESHOLD * SLEEP_ANGULAR_THRESHOLD)
	{
		sleepTimer += dt;
		if (sleepTimer >= SLEEP_TIME_THRESHOLD)
			SetSleeping(true);
	}
	else
	{
		sleepTimer = 0.f;
	}
}

// ── 기존 구현 ──

void RigidBody::SetPosition(const Vec3& Pos)
{
	positionWorld = Pos;
	WakeUp();
}

void RigidBody::SetOrientation(float pitchDeg, float yawDeg, float rollDeg)
{
	orientation = QuatFromEuler(pitchDeg, yawDeg, rollDeg);
	WakeUp();
}

void RigidBody::SetOrientation(const Quat& quat)
{
	orientation = quat;
	WakeUp();
}

void RigidBody::SetBodyType(BodyType type)
{
	bodyType = type;

	if (type != BodyType::Dynamic)
	{
		invMass = 0.f;
		invInertiaLocal.MakeZero();
	}
}

void RigidBody::SetMass(float m)
{
	if (bodyType == BodyType::Dynamic)
		invMass = (m > 0.f) ? 1.f / m : 0.f;
}

void RigidBody::SetInertia(const Mat3& matInertia)
{
	invInertiaLocal = Mat3Inverse(matInertia);
	UpdateWorldInertia();
}

void RigidBody::SetRestitution(float r)
{
	restitution = r;
}

void RigidBody::SetFriction(float f)
{
	friction = f;
}

void RigidBody::SetLinearDamping(float d)
{
	linearDamping = d;
}

void RigidBody::SetAngularDamping(float d)
{
	angularDamping = d;
}

void RigidBody::ApplyForce(const Vec3& force)
{
	WakeUp();
	forceAccum += force;
}

void RigidBody::ApplyForceAtPoint(const Vec3& force, const Vec3& point)
{
	WakeUp();
	forceAccum += force;

	torqueAccum += Vec3Cross(point - positionWorld, force);
}

void RigidBody::ApplyImpulseAtPoint(const Vec3& impulse, const Vec3& point)
{
	WakeUp();
	linearVelocity += impulse * invMass; // dv = J / m
	angularVelocity += invInertiaWorld * Vec3Cross(point - positionWorld, impulse); // dw = L / I(inertia)
}

Vec3 RigidBody::GetVelocityAtPoint(const Vec3& point) const
{
	return linearVelocity + Vec3Cross(angularVelocity, point - positionWorld);
}

void RigidBody::ApplyPseudoImpulseAtPoint(const Vec3& impulse, const Vec3& point)
{
	pseudoLinearVelocity += impulse * invMass;
	pseudoAngularVelocity += invInertiaWorld * Vec3Cross(point - positionWorld, impulse);
}

Vec3 RigidBody::GetPseudoVelocityAtPoint(const Vec3& point) const
{
	return pseudoLinearVelocity + Vec3Cross(pseudoAngularVelocity, point - positionWorld);
}

void RigidBody::ClearPseudoVelocities()
{
	pseudoLinearVelocity.MakeZero();
	pseudoAngularVelocity.MakeZero();
}

void RigidBody::UpdateWorldInertia()
{
	Mat3 r = QuatToMat3(orientation);
	invInertiaWorld = r * invInertiaLocal * Mat3Transpose(r);
}

void RigidBody::IntegrateVelocity(float dt)
{
	if (bodyType != BodyType::Dynamic)
		return;
	if (isSleeping)
		return;

	Vec3 a_lin = forceAccum * invMass;
	Vec3 a_ang = invInertiaWorld * torqueAccum;

	linearVelocity += a_lin * dt;
	angularVelocity += a_ang * dt;

	linearVelocity *= powf(1.f - linearDamping, dt);
	angularVelocity *= powf(1.f - angularDamping, dt);
}

void RigidBody::IntegratePosition(float dt)
{
	if (bodyType != BodyType::Dynamic)
		return;
	if (isSleeping)
		return;

	// 속도 클램프 — 과도한 충돌 반응으로 인한 폭발 방지
	float linSpeedSq = Vec3LengthSq(linearVelocity);
	if (linSpeedSq > maxLinearSpeed * maxLinearSpeed)
		linearVelocity *= maxLinearSpeed / sqrtf(linSpeedSq);

	float angSpeedSq = Vec3LengthSq(angularVelocity);
	if (angSpeedSq > maxAngularSpeed * maxAngularSpeed)
		angularVelocity *= maxAngularSpeed / sqrtf(angSpeedSq);
	Vec3 totalLinVel = linearVelocity + pseudoLinearVelocity;
	Vec3 totalAngVel = angularVelocity + pseudoAngularVelocity;

	// 1. 위치 업데이트
	positionWorld += totalLinVel * dt;

	// 2. 회전 업데이트 (더 정밀한 Delta Rotation 방식)
	float angSpeed = Vec3Length(totalAngVel);
	if (angSpeed > 1e-9f) // 아주 미세한 속도도 허용
	{
		Vec3 axis = totalAngVel / angSpeed;
		float angle = angSpeed * dt;

		// 델타 쿼터니언 생성
		Quat deltaRot = QuatFromAxisAngle(axis, angle);

		// 기존 회전에 적용 (World Space 방식)
		orientation = QuatNormalize(deltaRot * orientation);
	}

	//// 3. 다음 프레임을 위한 월드 관성 갱신
	UpdateWorldInertia();

	//// 4. Pseudo 속도 초기화
	//ClearPseudoVelocities();
}

void RigidBody::ClearAccumulators()
{
	forceAccum.MakeZero();
	torqueAccum.MakeZero();
}

void RigidBody::SetOrientationTarget(const Quat& target, float kp, float kd)
{
	WakeUp();
	hasOrientationTarget = true;
	orientationTarget = target;
	orientationInjectionKp = kp;
	orientationInjectionKd = kd;
}

void RigidBody::ClearOrientationTarget()
{
	hasOrientationTarget = false;
}

void RigidBody::ApplyOrientationInjection()
{
	if (!hasOrientationTarget)
		return;

	// 목표 회전, 현재 회전의 오차
	Quat q_error = orientationTarget * QuatConjugate(orientation);

	if (0.f > q_error.w)
		q_error = Quat(-q_error.x, -q_error.y, -q_error.z, -q_error.w);

	Vec3 outAxis;
	float outAngle;

	QuatToAxisAngle(q_error, outAxis, outAngle);

	// outAxis : 오차에 의한 축
	// outAngle : 오차 각도
	// orientationInjectionKp : 비례 이득 상수 
	// angular velocity : 현재 각속도
	// orientationInjectionKd : 감쇠 상수
	Vec3 torque = outAxis * (outAngle * orientationInjectionKp) -
		angularVelocity * orientationInjectionKd;

	// Torque 적용
	torqueAccum += torque;
}


