#include "PhysicsPch.h"
#include "NarrowPhase.h"
#include "RigidBody.h"
#include "Collision.h"

void NarrowPhase::TestPair(const ColliderPair& pair, std::vector<Contact>& contacts, std::vector<Contact>& triggers)
{
	Collider* a = pair.colA;
	Collider* b = pair.colB;

	// 순서 맞추기 (Sphere < Box < Plane)
	if (a->type > b->type)
		std::swap(a, b); 

	std::vector<Contact> temp;

	switch (a->type)
	{
	case Collider::ColliderType::Sphere:
		switch (b->type)
		{
		case Collider::ColliderType::Sphere:
			SphereSphere(static_cast<SphereCollider*>(a), static_cast<SphereCollider*>(b), temp);
			break;
		case Collider::ColliderType::Box:
			SphereBox(static_cast<SphereCollider*>(a), static_cast<BoxCollider*>(b), temp);
			break;
		case Collider::ColliderType::Plane:
			SpherePlane(static_cast<SphereCollider*>(a), static_cast<PlaneCollider*>(b), temp);
			break;
		}
		break;
	case Collider::ColliderType::Box:
		switch (b->type)
		{
		case Collider::ColliderType::Box:
			BoxBox(static_cast<BoxCollider*>(a), static_cast<BoxCollider*>(b), temp);
			break;
		case Collider::ColliderType::Plane:
			BoxPlane(static_cast<BoxCollider*>(a), static_cast<PlaneCollider*>(b), temp);
			break;
		}
		break;
	}

	bool isTrigger = a->isTrigger || b->isTrigger;

	for (auto& ct : temp)
	{
		ct.colA = a;
		ct.colB = b;
		ct.bodyA = a->body;
		ct.bodyB = b->body;

		if (isTrigger)
			triggers.push_back(ct);
		else
			contacts.push_back(ct);
	}
}
