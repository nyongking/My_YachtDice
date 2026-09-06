#pragma once

#include "Collider.h"
#include "Contact.h"
#include <vector>

class NarrowPhase
{
public:
	void TestPair(const ColliderPair& pair, 
		std::vector<Contact>& contacts,
		std::vector<Contact>& triggers);
};

