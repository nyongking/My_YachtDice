#pragma once
#include "Contact.h"
#include <vector>

class RigidBody;

struct Island
{
	std::vector<RigidBody*> bodies;
	std::vector<Contact*> contacts;
};

class IslandBuilder
{
public:
	void Build(std::vector<RigidBody*>& bodies, std::vector<Contact>& contacts);

	const std::vector<Island>& GetIslands() const { return m_islands; }


private:
	int Find(int i);
	void Unite(int a, int b);

	std::vector<int> m_parent;
	std::vector<int> m_rank;
	std::vector<Island> m_islands;
};

