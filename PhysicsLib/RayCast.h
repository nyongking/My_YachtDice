#pragma once
#include "Ray.h"
#include "Collider.h"

// Ray-AABB intersection (Slab method)
bool RayIntersectAABB(const Ray& ray, const AABB& aabb, float& tMin);

// Ray-Sphere intersection
bool RayIntersectSphere(const Ray& ray, const Vec3& center, float radius, float& t);

// Ray-OBB intersection (local space transform + Slab method)
bool RayIntersectOBB(const Ray& ray, const Vec3& center,
	const Vec3& halfExtents, const Quat& orientation, float& t);
