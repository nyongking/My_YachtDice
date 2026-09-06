#include "PhysicsPch.h"
#include "Collision.h"
#include "Collider.h"
#include "RigidBody.h"
#include "Contact.h"

void SphereSphere(const SphereCollider* a, const SphereCollider* b, std::vector<Contact>& out)
{
	Vec3 aPos = a->GetWorldPosition();
	Vec3 diff = b->GetWorldPosition() - aPos;

	float dist = Vec3Length(diff);

	// Sphere radius의 합이 Sphere 중심 사이의 거리보다 크다 -> 충돌
	if (dist < a->radius + b->radius)
	{
		Contact ct;
		ct.normal = diff / dist;
		ct.depth = (a->radius + b->radius) - dist;
		ct.point = aPos + ct.normal * a->radius;

		out.push_back(std::move(ct));
	}
}

void SpherePlane(const SphereCollider* a, const PlaneCollider* b, std::vector<Contact>& out)
{
	Vec3 center = a->GetWorldPosition();

	float signedDist = Vec3Dot(b->normal, center) - b->dist;

	float depth = a->radius - signedDist;

	// Sphere과 Plane 사이의 가장 가까운 점과 Plane 까지의 거리 vs Sphere의 radius
	if (0.f < depth)
	{
		Contact ct;
		ct.normal = -(b->normal);       // A→B (sphere→plane)
		ct.depth = depth;
		ct.point = center - b->normal * a->radius;  // 구 표면의 평면 쪽 접촉점

		out.push_back(std::move(ct));
	}

}

void SphereBox(const SphereCollider* a, const BoxCollider* b, std::vector<Contact>& out)
{
	Vec3 sphereCenter = a->GetWorldPosition();
	Vec3 boxCenter = b->GetWorldPosition();
	Mat3 boxRot = QuatToMat3(b->GetWorldOrientation());
	Vec3 half = b->halfExtents;

	// 가장 가까운 점 찾기 방식
	// 1. Sphere의 중심 -> 박스의 로컬 좌표로 변환
	Vec3 diff = sphereCenter - boxCenter;
	Vec3 localCenter = Vec3(
		Vec3Dot(diff, Vec3(boxRot.m[0][0], boxRot.m[0][1], boxRot.m[0][2])),
		Vec3Dot(diff, Vec3(boxRot.m[1][0], boxRot.m[1][1], boxRot.m[1][2])),
		Vec3Dot(diff, Vec3(boxRot.m[2][0], boxRot.m[2][1], boxRot.m[2][2]))
	);

	// 2. 위의 로컬 좌표를 클램핑 (-half.axislength ~ half.axislength)
	Vec3 close;

	// localCenter.x가 half.x보다 크다 -> half.x,
	// localCenter.x가 -half.x보다 작다 -> -half.x
	close.x = fmaxf(-half.x, fminf(localCenter.x, half.x));
	close.y = fmaxf(-half.y, fminf(localCenter.y, half.y));
	close.z = fmaxf(-half.z, fminf(localCenter.z, half.z));

	Vec3 localDiff = localCenter - close;
	float distSq = Vec3LengthSq(localDiff); // 거리 제곱

	if (0.0001f < distSq) // 구 중심이 박스의 바깥에 있을 경우
	{
		float dist = sqrtf(distSq);
		if (dist >= a->radius)
			return; // 충돌 x

		// 로컬 법선 -> 월드 법선 (box→sphere 방향)
		Vec3 localNormal = localDiff / dist;
		Vec3 normal = boxRot * localNormal;

		Contact ct;
		ct.normal = -normal;             // A→B (sphere→box)
		ct.depth = a->radius - dist;
		ct.point = sphereCenter - normal * a->radius;
		out.push_back(std::move(ct));
	}
	else // 구 중심이 박스 안에 있는 경우 (완전히 파고든 경우)
	{
		float minDist = FLOAT__MAX;
		int minAxis = 0;
		float minSign = 1.f;

		for (int i = 0; i < 3; ++i)
		{
			float distPos = half.v[i] - localCenter.v[i];
			float distNeg = half.v[i] + localCenter.v[i];

			if (distPos < distNeg)
			{
				if (distPos < minDist)
				{
					minDist = distPos;
					minAxis = i;
					minSign = 1.f;
				}
			}
			else
			{
				if (distNeg < minDist)
				{
					minDist = distNeg;
					minAxis = i;
					minSign = -1.f;
				}
			}
		}

		Vec3 localNormal; // 가장 가까운 면의 normal을 월드화 (box→sphere 방향)
		localNormal.v[minAxis] = minSign;
		Vec3 normal = boxRot * localNormal;

		Contact ct;
		ct.normal = -normal;             // A→B (sphere→box)
		ct.depth = minDist + a->radius;
		ct.point = sphereCenter - normal * a->radius;
		out.push_back(std::move(ct));
	}

}

void BoxBox(const BoxCollider* a, const BoxCollider* b, std::vector<Contact>& out)
{
	Vec3 posA = a->GetWorldPosition();
	Vec3 posB = b->GetWorldPosition();

	Vec3 halfA = a->halfExtents;
	Vec3 halfB = b->halfExtents;

	Mat3 rotA = QuatToMat3(a->GetWorldOrientation());
	Mat3 rotB = QuatToMat3(b->GetWorldOrientation());

	Vec3 axisA[3] = {
		rotA.row0,
		rotA.row1,
		rotA.row2
	};

	Vec3 axisB[3] = {
		rotB.row0,
		rotB.row1,
		rotB.row2
	};

	Vec3 diff = posB - posA;

	float dot[3][3];
	float absDot[3][3];

	const float epsilon = 1e-6f;

	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			dot[i][j] = Vec3Dot(axisA[i], axisB[j]);
			absDot[i][j] = fabsf(dot[i][j]) + epsilon;
		}
	}

	// diff를 axisA에 투영
	float dA[3] = {
		Vec3Dot(diff, axisA[0]),
		Vec3Dot(diff, axisA[1]),
		Vec3Dot(diff, axisA[2])
	};

	float	minOverlap = FLOAT__MAX;
	int		minAxisIndex = -1;
	Vec3	minAxisDir;

	// 15축 중 3축 (a의 축 3개, index : 0~2)
	for (int i = 0; i < 3; ++i)
	{
		float rA = halfA.v[i];
		float rB = halfB.v[0] * absDot[i][0]
			+ halfB.v[1] * absDot[i][1]
			+ halfB.v[2] * absDot[i][2];
		float dist = fabsf(dA[i]);
		float overlap = rA + rB - dist;

		if (0.f > overlap)
			return; // 해당 축에서 충돌이 없음(분리축)!
		if (overlap < minOverlap)
		{
			minOverlap = overlap;
			minAxisIndex = i;
			minAxisDir = axisA[i];

			if (dA[i] < 0.f)
				minAxisDir = -minAxisDir;
		}
	}

	float dB[3] = {
		Vec3Dot(diff, axisB[0]),
		Vec3Dot(diff, axisB[1]),
		Vec3Dot(diff, axisB[2])
	};

	// 15축 중 3축 (b의 축 3개, index : 3~5)
	for (int i = 0; i < 3; ++i)
	{
		float rA = halfA.v[0] * absDot[0][i]
			+ halfA.v[1] * absDot[1][i]
			+ halfA.v[2] * absDot[2][i];
		float rB = halfB.v[i];
		float dist = fabsf(dB[i]);
		float overlap = rA + rB - dist;

		if (overlap < 0.f)
			return; // 해당 축에서 충돌 없음

		if (overlap < minOverlap)
		{
			minOverlap = overlap;
			minAxisIndex = i + 3; // 인덱스는 3부터 시작
			minAxisDir = axisB[i];

			if (dB[i] < 0.f)
				minAxisDir = -minAxisDir;
		}
	}

	// 15축 중 9축(a[i] x b[j], index : 6~14)
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			Vec3 cross = Vec3Cross(axisA[i], axisB[j]);
			float len = Vec3Length(cross);

			if (len < 1e-6f) // 평행축일 경우
				continue;

			cross /= len; // normalize

			float rA = 0.f;
			float rB = 0.f;
			for (int k = 0; k < 3; ++k)
			{
				rA += halfA.v[k] * fabsf(Vec3Dot(axisA[k], cross));
				rB += halfB.v[k] * fabsf(Vec3Dot(axisB[k], cross));
			}

			float dist = fabsf(Vec3Dot(diff, cross));
			float overlap = rA + rB - dist;

			if (overlap < 0.f)
				return; // 충돌 없음

			if (overlap < minOverlap)
			{
				minOverlap = overlap;
				minAxisIndex = 6 + i * 3 + j;
				minAxisDir = cross;
				if (Vec3Dot(diff, cross) < 0.f)
					minAxisDir = -minAxisDir;
			}
		}
	}

	// 접촉점 생성...

	if (minAxisIndex < 6) // a 혹은 b의 축
	{
		// 면-면, 면-엣지 접촉
		// 박스의 꼭짓점 중 면에 가장 깊이 파고든 것이 접촉점

		const BoxCollider* faceBox;
		const BoxCollider* otherBox;
		Vec3 faceHalf, otherHalf;
		Vec3 faceAxis[3], otherAxis[3];
		Vec3 faceCenter, otherCenter;

		if (minAxisIndex < 3) // a
		{
			faceBox = a; otherBox = b;
			faceHalf = halfA; otherHalf = halfB;
			faceCenter = posA; otherCenter = posB;

			for (int i = 0; i < 3; ++i)
			{
				faceAxis[i] = axisA[i];
				otherAxis[i] = axisB[i];
			}
		}
		else // b
		{
			faceBox = b; otherBox = a;
			faceHalf = halfB; otherHalf = halfA;
			faceCenter = posB; otherCenter = posA;

			for (int i = 0; i < 3; ++i)
			{
				faceAxis[i] = axisB[i];
				otherAxis[i] = axisA[i];
			}
		}

		int faceAxisIdx = (minAxisIndex < 3) ? minAxisIndex : (minAxisIndex - 3);

		// faceBox→otherBox 방향 계산
		// minAxisDir는 항상 A→B 방향.
		// faceBox=A(minAxisIndex<3)이면 minAxisDir가 face→other 방향 ✓
		// faceBox=B(minAxisIndex>=3)이면 -minAxisDir가 face→other 방향
		Vec3 faceDir = (minAxisIndex < 3) ? minAxisDir : -minAxisDir;

		Vec3 vertices[8];
		for (int i = 0; i < 8; ++i)
		{
			Vec3 local;
			local.x = (i & 1) ? otherHalf.x : -otherHalf.x;
			local.y = (i & 2) ? otherHalf.y : -otherHalf.y;
			local.z = (i & 4) ? otherHalf.z : -otherHalf.z;

			vertices[i] = otherCenter
				+ otherAxis[0] * local.x
				+ otherAxis[1] * local.y
				+ otherAxis[2] * local.z;
		}

		Vec3 facePoint = faceCenter + faceDir * faceHalf.v[faceAxisIdx];

		for (int i = 0; i < 8; ++i)
		{
			Vec3 toVert = vertices[i] - facePoint;
			float penetration = -Vec3Dot(toVert, faceDir);

			if (0.f < penetration)
			{
				Contact ct;
				ct.normal = minAxisDir;
				ct.depth = penetration;
				ct.point = vertices[i];

				out.push_back(std::move(ct));
			}
		}
	}
	else // 엣지-엣지 접촉
	{
		int edgeA = (minAxisIndex - 6) / 3;
		int edgeB = (minAxisIndex - 6) % 3;

		// a 엣지
		Vec3 pointA = posA;
		for (int i = 0; i < 3; ++i)
		{
			if (i == edgeA)
				continue;
			float sign = (Vec3Dot(axisA[i], diff) > 0.f) ? 1.f : -1.f;
			pointA += axisA[i] * (halfA.v[i] * sign);
		}

		// b 엣지
		Vec3 pointB = posB;
		for (int i = 0; i < 3; ++i)
		{
			if (i == edgeB)
				continue;
			float sign = (Vec3Dot(axisB[i], diff) < 0.f) ? 1.f : -1.f;
			pointB += axisB[i] * (halfB.v[i] * sign);
		}

		Vec3 dirA = axisA[edgeA];
		Vec3 dirB = axisB[edgeB];
		Vec3 w = pointA - pointB;

		float a11 = Vec3Dot(dirA, dirA);
		float a12 = Vec3Dot(dirA, dirB);
		float a22 = Vec3Dot(dirB, dirB);
		float b1 = Vec3Dot(dirA, w);
		float b2 = Vec3Dot(dirB, w);

		float denom = a11 * a22 - a12 * a12;
		float tA = 0.f, tB = 0.f;
		if (fabsf(denom) > 1e-8f) // 0 나누기 방지
		{
			tA = (a12 * b2 - a22 * b1) / denom;
			tB = (a11 * b2 - a12 * b1) / denom;
		}

		tA = fmaxf(-halfA.v[edgeA], fminf(tA, halfA.v[edgeA]));
		tB = fmaxf(-halfB.v[edgeB], fminf(tB, halfB.v[edgeB]));

		Vec3 closeA = pointA + dirA * tA;
		Vec3 closeB = pointB + dirB * tB;

		Contact ct;
		ct.normal = minAxisDir;
		ct.depth = minOverlap;
		ct.point = (closeA + closeB) * 0.5f; // A, B의 중간지점
		out.push_back(std::move(ct));
	}

}

void BoxPlane(const BoxCollider* a, const PlaneCollider* b, std::vector<Contact>& out)
{
	Vec3 boxCenter = a->GetWorldPosition();
	Mat3 boxRot = QuatToMat3(a->GetWorldOrientation());
	Vec3 half = a->halfExtents;

	Vec3 axis[3] = {
		Vec3(boxRot.row0),
		Vec3(boxRot.row1),
		Vec3(boxRot.row2)
	};

	for (int i = 0; i < 8; ++i)
	{
		Vec3 local;
		local.x = (i & 1) ? half.x : -half.x;
		local.y = (i & 2) ? half.y : -half.y;
		local.z = (i & 4) ? half.z : -half.z;

		Vec3 vertex = boxCenter
			+ axis[0] * local.x
			+ axis[1] * local.y
			+ axis[2] * local.z;

		float signedDist = Vec3Dot(b->normal, vertex) - b->dist;

		if (0.f > signedDist) // 평면 관통
		{
			Contact ct;

			ct.normal = -(b->normal);    // A→B (box→plane)
			ct.depth = -signedDist;
			ct.point = vertex;

			out.push_back(std::move(ct));
		}
	}
}
