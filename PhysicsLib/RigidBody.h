#pragma once
#include "Vec3.h"
#include "Quat.h"
#include "Mat3.h"
#include "Operation.h"

class RigidBody
{
public:
	enum class BodyType { Static, Kinematic, Dynamic };
	// Setter----------------------------------
	void		SetPosition(const Vec3& Pos);
	void		SetOrientation(float pitchDeg, float yawDeg, float rollDeg);
	void		SetOrientation(const Quat& orientation);

	void		SetBodyType(BodyType type);
	void		SetMass(float mass);
	void		SetInertia(const Mat3& matInertia);
	void		SetRestitution(float r);
	void		SetFriction(float f);
	void		SetLinearDamping(float d);
	void		SetAngularDamping(float d);

	// Getter----------------------------------
	BodyType	GetBodyType() const { return bodyType; }
	Vec3		GetPosition() const { return positionWorld; }
	Quat		GetOrientation() const { return orientation; }

	Vec3		GetLinearVelocity() const { return linearVelocity; }
	Vec3		GetAngularVelocity() const { return angularVelocity; }

	void		SetLinearVelocity(const Vec3& v) { if (Vec3LengthSq(v) > 0.f) WakeUp(); linearVelocity = v; }
	void		SetAngularVelocity(const Vec3& v) { if (Vec3LengthSq(v) > 0.f) WakeUp(); angularVelocity = v; }

	float		GetInvMass() const { return invMass; }
	Mat3		GetInvInertiaWorld() const { return invInertiaWorld; }
	float		GetRestitution() const { return restitution; }
	float		GetFriction() const { return friction; }

	bool		IsStatic() const { return bodyType == BodyType::Static; }
	bool		IsKinematic() const { return bodyType == BodyType::Kinematic; }
	bool		IsDynamic() const { return bodyType == BodyType::Dynamic; }

	// Sleep 시스템
	bool		IsSleeping() const { return isSleeping; }
	float		GetSleepTimer() const { return sleepTimer; }
	void		SetSleeping(bool sleep);
	void		WakeUp();
	void		UpdateSleepState(float dt);
	void		ResetSleepTimer() { sleepTimer = 0.f; }

	static constexpr float SLEEP_LINEAR_THRESHOLD  = 0.25f;
	static constexpr float SLEEP_ANGULAR_THRESHOLD = 0.25f;
	// 고정 스텝이 1/60(0.0167)이라 이 값이 dt보다 작으면 한 스텝 만에 잠든다.
	// 모서리로 넘어가는 도중 각속도가 0을 지나는 순간을 붙잡아 기울어진 채 굳어버리므로,
	// 실제로 "한동안 정지해 있었다"를 의미하는 길이로 둔다.
	static constexpr float SLEEP_TIME_THRESHOLD    = 0.1f;

	// ----------------------------------
	//
	// 질량 중심에 힘 적용 ->  forceAccum 변경
	void		ApplyForce(const Vec3& force);

	// 특정 지점에 point 적용 -> forceAccum, torqueAccum 변경
	void		ApplyForceAtPoint(const Vec3& force, const Vec3& point);

	// 특정 지점에 충격량(J = F * t) 적용, -> linearVelocity, angularVelocity 변경 (J = m * dV)
	void		ApplyImpulseAtPoint(const Vec3& impulse, const Vec3& point);

	// 특정 지점의 속도 구하기
	Vec3		GetVelocityAtPoint(const Vec3& point) const;

	// Split Impulse: 위치 보정용 pseudo velocity(가짜 속도)
	void		ApplyPseudoImpulseAtPoint(const Vec3& impulse, const Vec3& point);
	Vec3		GetPseudoVelocityAtPoint(const Vec3& point) const;
	void		ClearPseudoVelocities();

	// 회전 상황을 기준으로 Mass Inertia 업데이트,
	void		UpdateWorldInertia();

	// 적분 (Semi-implicit Euler: 속도 먼저, 위치 나중에)
	void		IntegrateVelocity(float dt);
	void		IntegratePosition(float dt);

	// accum = 0
	void		ClearAccumulators();

	// Orientation Injection
	void		SetOrientationTarget(const Quat& target, float kp, float kd);
	void		ClearOrientationTarget();

	void		ApplyOrientationInjection();


private:
	BodyType	bodyType = BodyType::Static;
	Vec3		positionWorld;
	Quat		orientation;

	Vec3		linearVelocity;
	Vec3		angularVelocity;

	float		invMass = 0.f;
	Mat3		invInertiaLocal;
	Mat3		invInertiaWorld;

	float		restitution = 0.5f;
	float		friction = 0.5f;

	float		linearDamping = 0.01f;
	float		angularDamping = 0.01f;

	float		maxLinearSpeed = 300'000'000.f;
	float		maxAngularSpeed = 300.f;

	Vec3		forceAccum;
	Vec3		torqueAccum;

	// Split Impulse : 위치 보정 전용 (매 Step 후 클리어)
	Vec3		pseudoLinearVelocity;
	Vec3		pseudoAngularVelocity;

	// Sleep 시스템
	bool		isSleeping = false;
	float		sleepTimer = 0.f;

	// Orientation Injection : 원하는 만큼 물리적인 회전을 적용하기 위한 값,
	bool		hasOrientationTarget = false;
	Quat		orientationTarget;
	float		orientationInjectionKp = 50.f; // stiffness
	float		orientationInjectionKd = 5.f; // damping
};
