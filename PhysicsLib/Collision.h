#pragma once
#include <vector>
#include "Contact.h"

void SphereSphere(const struct SphereCollider* a, const struct SphereCollider* b, std::vector<Contact>& out);
void SpherePlane(const struct SphereCollider* a, const struct PlaneCollider* b, std::vector<Contact>& out);
void SphereBox(const struct SphereCollider* a, const struct BoxCollider* b, std::vector<Contact>& out);

void BoxBox(const struct BoxCollider* a, const struct BoxCollider* b, std::vector<Contact>& out);
void BoxPlane(const struct BoxCollider* a, const struct PlaneCollider* b, std::vector<Contact>& out);