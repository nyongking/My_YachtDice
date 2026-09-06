#include "PhysicsPch.h"
#include "RayCast.h"
#include "Operation.h"

#include <cmath>
#include <algorithm>

// Ray-AABB (Slab Method)
bool RayIntersectAABB(const Ray& ray, const AABB& aabb, float& tMin)
{
	float tNear = -FLT_MAX;
	float tFar  =  FLT_MAX;

	const float* origin = ray.origin.v;
	const float* dir    = ray.direction.v;
	const float* bmin   = aabb.Min.v;
	const float* bmax   = aabb.Max.v;

	for (int i = 0; i < 3; ++i)
	{
		if (fabsf(dir[i]) < 1e-8f)
		{
			if (origin[i] < bmin[i] || origin[i] > bmax[i])
				return false;
		}
		else
		{
			float invD = 1.f / dir[i];
			float t1 = (bmin[i] - origin[i]) * invD;
			float t2 = (bmax[i] - origin[i]) * invD;

			if (t1 > t2) std::swap(t1, t2);

			tNear = std::max(tNear, t1);
			tFar  = std::min(tFar,  t2);

			if (tNear > tFar || tFar < 0.f)
				return false;
		}
	}

	tMin = tNear > 0.f ? tNear : tFar;
	return tMin >= 0.f;
}

// Ray-Sphere (quadratic formula)
bool RayIntersectSphere(const Ray& ray, const Vec3& center, float radius, float& t)
{
	Vec3 oc = ray.origin - center;

	float a = Vec3Dot(ray.direction, ray.direction);
	float b = 2.f * Vec3Dot(oc, ray.direction);
	float c = Vec3Dot(oc, oc) - radius * radius;

	float discriminant = b * b - 4.f * a * c;
	if (discriminant < 0.f)
		return false;

	float sqrtD = sqrtf(discriminant);
	float t0 = (-b - sqrtD) / (2.f * a);
	float t1 = (-b + sqrtD) / (2.f * a);

	if (t0 >= 0.f)
	{
		t = t0;
		return true;
	}
	if (t1 >= 0.f)
	{
		t = t1;
		return true;
	}

	return false;
}

// Ray-OBB (transform to local space + Slab Method)
bool RayIntersectOBB(const Ray& ray, const Vec3& center,
	const Vec3& halfExtents, const Quat& orientation, float& t)
{
	Mat3 rot = QuatToMat3(orientation);
	Vec3 axes[3] = { rot.rows[0], rot.rows[1], rot.rows[2] };

	Vec3 delta = ray.origin - center;

	Vec3 localOrigin;
	Vec3 localDir;

	for (int i = 0; i < 3; ++i)
	{
		localOrigin.v[i] = Vec3Dot(delta, axes[i]);
		localDir.v[i]    = Vec3Dot(ray.direction, axes[i]);
	}

	float tNear = -FLT_MAX;
	float tFar  =  FLT_MAX;

	for (int i = 0; i < 3; ++i)
	{
		if (fabsf(localDir.v[i]) < 1e-8f)
		{
			if (localOrigin.v[i] < -halfExtents.v[i] || localOrigin.v[i] > halfExtents.v[i])
				return false;
		}
		else
		{
			float invD = 1.f / localDir.v[i];
			float t1 = (-halfExtents.v[i] - localOrigin.v[i]) * invD;
			float t2 = ( halfExtents.v[i] - localOrigin.v[i]) * invD;

			if (t1 > t2) std::swap(t1, t2);

			tNear = std::max(tNear, t1);
			tFar  = std::min(tFar,  t2);

			if (tNear > tFar || tFar < 0.f)
				return false;
		}
	}

	t = tNear > 0.f ? tNear : tFar;
	return t >= 0.f;
}
