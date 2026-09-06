#include "PhysicsPch.h"
#include "IslandBuilder.h"
#include "RigidBody.h"
#include <unordered_map>

void IslandBuilder::Build(std::vector<RigidBody*>& bodies, std::vector<Contact>& contacts)
{
	m_islands.clear();

	if (bodies.empty())
		return;

	std::unordered_map<RigidBody*, int> bodyIndex;
	for (int i = 0; i < static_cast<int>(bodies.size()); ++i)
		bodyIndex[bodies[i]] = i;

	int n = static_cast<int>(bodies.size());
	m_parent.resize(n);
	m_rank.resize(n);
	
	for (int i = 0; i < n; ++i)
	{
		m_parent[i] = i;
		m_rank[i] = 0;
	}

	for (auto& ct : contacts)
	{
		if (nullptr == ct.bodyA || nullptr == ct.bodyB)
			continue;

		bool aFixed = !ct.bodyA->IsDynamic();
		bool bFixed = !ct.bodyB->IsDynamic();

		if (aFixed && bFixed)
			continue;

		auto iterA = bodyIndex.find(ct.bodyA);
		auto iterB = bodyIndex.find(ct.bodyB);

		if (bodyIndex.end() == iterA || bodyIndex.end() == iterB)
			continue;

		Unite(iterA->second, iterB->second);
	}

	std::unordered_map<int, int> rootToisland;

	for (int i = 0; i < n; ++i)
	{
		if (!bodies[i]->IsDynamic())
			continue;

		int root = Find(i);
		auto it = rootToisland.find(root);
		if (it == rootToisland.end())
		{
			rootToisland[root] = static_cast<int>(m_islands.size());
			m_islands.push_back(Island{});
		}
		m_islands[rootToisland[root]].bodies.push_back(bodies[i]);
	}

	for (auto& ct : contacts)
	{
		if (nullptr == ct.bodyA || nullptr == ct.bodyB)
			continue;

		RigidBody* body = ct.bodyA->IsDynamic() ? ct.bodyA : ct.bodyB;

		if (!body->IsDynamic())
			continue;

		auto itIdx = bodyIndex.find(body);
		if (itIdx == bodyIndex.end())
			continue;

		int root = Find(itIdx->second);
		auto itIsland = rootToisland.find(root);
		if (itIsland == rootToisland.end())
			continue;

		m_islands[itIsland->second].contacts.push_back(&ct);
	}

}

int IslandBuilder::Find(int i)
{
	while (m_parent[i] != i)
	{
		m_parent[i] = m_parent[m_parent[i]];
		i = m_parent[i];
	}
	return i;
}

void IslandBuilder::Unite(int a, int b)
{
	int valueA = Find(a);
	int valueB = Find(b);

	if (m_rank[valueA] < m_rank[valueB])
		std::swap(valueA, valueB);
	m_parent[valueB] = valueA;
	
	if (m_rank[valueA] == m_rank[valueB])
		++m_rank[valueA];
}
