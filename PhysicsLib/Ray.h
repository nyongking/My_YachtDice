#pragma once
#include "Vec3.h"

struct Collider;

struct Ray
{
	Vec3         origin;
	Vec3         direction;                // normalized direction
	unsigned int layerMask = 0xFFFFFFFF;   // only test layers in this mask
};

struct RayHit
{
	float     distance = 0.f;
	Vec3      point;
	Collider* collider = nullptr;
};
