#pragma once
#include "Vec3.h"
#include "Quat.h"

struct AABB
{
	AABB() {}
	AABB(Vec3 Min, Vec3 Max) : Min(Min), Max(Max) {}

	Vec3 Min;
	Vec3 Max;
};

struct Collider
{
	enum class ColliderType
	{
		Sphere,
		Box,
		Plane
	};

	Collider(ColliderType type) : type(type) {}
	virtual ~Collider() = default;

	virtual AABB GetAABB() const = 0;
	Vec3		 GetWorldPosition() const;
	Quat		 GetWorldOrientation() const;

	
#pragma region member
	const ColliderType	type;
	bool				enabled = true;
	bool				isTrigger = false;
	unsigned int		layer = 1;
	unsigned int		layerMask = 0xFFFFFFFF;

	Vec3				staticPosition;
	Quat				staticOrientation;
	Vec3				posOffset;
	Quat				rotOffset;
	class RigidBody*	body = nullptr;
#pragma endregion
};

struct SphereCollider : public Collider
{
	SphereCollider() : Collider(ColliderType::Sphere) {}

	virtual AABB GetAABB() const override;


	float radius;
};

struct BoxCollider : public Collider
{
	BoxCollider() : Collider(ColliderType::Box) {}

	virtual AABB GetAABB() const override;


	Vec3 halfExtents;
};

struct PlaneCollider : public Collider
{
	PlaneCollider() : Collider(ColliderType::Plane) {}

	virtual AABB GetAABB() const override;

	Vec3 normal;
	float dist;
};

struct ColliderPair
{
	Collider* colA;
	Collider* colB;
};