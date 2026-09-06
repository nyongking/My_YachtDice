#include "PhysicsPch.h"
#include "Collider.h"

#include "RigidBody.h"

AABB SphereCollider::GetAABB() const
{
	Vec3 center = GetWorldPosition();
	return AABB(
		center - Vec3(radius, radius, radius),
		center + Vec3(radius, radius, radius)
	);
}

Vec3 Collider::GetWorldPosition() const
{
	if (body)
		return body->GetPosition() + QuatRotateVec3(body->GetOrientation(), posOffset);
	return staticPosition + posOffset;
}

Quat Collider::GetWorldOrientation() const
{
	if (body)
		return QuatMultiply(body->GetOrientation(), rotOffset);
	return QuatMultiply(staticOrientation, rotOffset);
}

AABB BoxCollider::GetAABB() const
{
	Vec3 center = GetWorldPosition();
	Mat3 rotMat = QuatToMat3(GetWorldOrientation());

	Vec3 absX = Vec3(fabsf(rotMat.m[0][0]), fabsf(rotMat.m[0][1]), fabsf(rotMat.m[0][2])) * halfExtents.x;
	Vec3 absY = Vec3(fabsf(rotMat.m[1][0]), fabsf(rotMat.m[1][1]), fabsf(rotMat.m[1][2])) * halfExtents.y;
	Vec3 absZ = Vec3(fabsf(rotMat.m[2][0]), fabsf(rotMat.m[2][1]), fabsf(rotMat.m[2][2])) * halfExtents.z;

	Vec3 extent = absX + absY + absZ;

	return AABB(center - extent, center + extent);
}

AABB PlaneCollider::GetAABB() const
{
	return AABB(Vec3(-FLOAT__MAX, -FLOAT__MAX, -FLOAT__MAX), 
		Vec3(FLOAT__MAX, FLOAT__MAX, FLOAT__MAX));
}
