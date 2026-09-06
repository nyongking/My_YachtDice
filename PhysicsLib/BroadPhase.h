#pragma once
#include <vector>
#include "Collider.h"

class BroadPhase
{
public:
	void GetPairsBruteForce(const std::vector<struct Collider*> colliders,
		std::vector<ColliderPair>& out);

private:
	bool LayerCollide(struct Collider* a, struct Collider* b);
	bool Overlaps(const AABB& a, const AABB& b);
};

