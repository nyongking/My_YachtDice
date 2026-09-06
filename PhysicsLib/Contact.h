#pragma once

#include "Vec3.h"

struct Contact
{
	Contact() = default;

	Vec3	point;
	Vec3	normal; // bodyA -> bodyB
	float	depth;
	struct Collider* colA = nullptr;
	struct Collider* colB = nullptr;
	class RigidBody* bodyA = nullptr;
	class RigidBody* bodyB = nullptr;

};

