#pragma once
#include "Contact.h"
#include <vector>

class ContactSolver
{
	struct ContactConstraint
	{
		Contact* ct;

		Vec3 rA;
		Vec3 rB;

		// normal (real velocity — restitution only)
		float effectiveMassNormal;
		float accumulatedImpulseN;
		float restitutionBias;

		// normal (pseudo velocity — position correction only)
		float baumgarteBias;
		float accumulatedPseudoN;

		// tangent (normal 기반 2축)
		Vec3 tangent1;
		Vec3 tangent2;
		float effectiveMassT1;
		float effectiveMassT2;
		float accumulatedImpulseT1;
		float accumulatedImpulseT2;

		float mixedFriction;
		float mixedRestitution;
	};

public:

	void Resolve(std::vector<Contact>& contacts, float dt);

	void SetIteration(int n) { iterations = n; }

private:

	void Presolve(std::vector<Contact>& contacts, float dt);
	void SolveVelocity();
	void SolvePositionBias();

	// normal에서 직교하는 tangent 2개 생성
	static void ComputeTangentBasis(const Vec3& normal, Vec3& t1, Vec3& t2);

	std::vector<ContactConstraint> m_constraints;

	int		iterations = 8;
	float	beta = 0.2f;  // Baumgarte stabilization (낮을수록 부드러운 보정)
	float	slop = 0.005f;

};
