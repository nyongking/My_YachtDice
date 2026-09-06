#include "PhysicsPch.h"
#include "BroadPhase.h"

void BroadPhase::GetPairsBruteForce(const std::vector<struct Collider*> colliders, std::vector<ColliderPair>& out)
{
	for (int i = 0; i < colliders.size(); ++i)
	{
		for (int j = i + 1; j < colliders.size(); ++j)
		{
			Collider* a = colliders[i];
			Collider* b = colliders[j];

			if (!a->enabled || !b->enabled)
				continue;

			// 1�ܰ� : �浹�ϰ��� �ϴ� Layer�� �ƴ�
			if (!LayerCollide(a, b))
				continue;

			// 2�ܰ� : Plane�� �ϴ� ������ Narrow Phase��
			if (a->type == Collider::ColliderType::Plane ||
				b->type == Collider::ColliderType::Plane)
			{
				out.push_back(ColliderPair{ a, b });
				continue;
			}

			// 3. AABB check
			if (Overlaps(a->GetAABB(), b->GetAABB()))
				out.push_back(ColliderPair{ a, b });
		}
	}
}

bool BroadPhase::LayerCollide(Collider* a, Collider* b)
{
	unsigned int bitA = (1u << a->layer);
	unsigned int bitB = (1u << b->layer);
	return (bitA & b->layerMask) && (bitB & a->layerMask);
}

bool BroadPhase::Overlaps(const AABB& a, const AABB& b)
{
	return ((a.Min.x <= b.Max.x && a.Max.x >= b.Min.x) && 
		(a.Min.y <= b.Max.y && a.Max.y >= b.Min.y) &&
		(a.Min.z <= b.Max.z && a.Max.z >= b.Min.z));
}
